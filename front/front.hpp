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
#include "bmpcache.hpp"
#include "cache.hpp"
#include "client_info.hpp"
#include "config.hpp"
#include "error.hpp"
#include "callback.hpp"
#include "colors.hpp"
#include "altoco.hpp"
#include "transport.hpp"

#include "RDP/GraphicUpdatePDU.hpp"
#include "RDP/capabilities/capset.hpp"

#include "RDP/gcc_conference_user_data/cs_core.hpp"
#include "RDP/gcc_conference_user_data/cs_sec.hpp"
#include "RDP/gcc_conference_user_data/cs_net.hpp"
#include "RDP/gcc_conference_user_data/cs_cluster.hpp"
#include "RDP/gcc_conference_user_data/cs_monitor.hpp"
#include "RDP/gcc_conference_user_data/sc_sec1.hpp"
#include "RDP/gcc_conference_user_data/sc_core.hpp"
#include "RDP/gcc_conference_user_data/sc_net.hpp"

#include "front_api.hpp"
#include "genrandom.hpp"

static inline int get_pixel(uint8_t* data, int x, int y, int width, int bpp)
{
    int pixels_per_byte = 8/bpp;
    int real_width = (width + pixels_per_byte-1) / pixels_per_byte;
    int start = y * real_width + x / pixels_per_byte;
    int shift = x & (pixels_per_byte-1);

    TODO(" this need some cleanup  but we should define unit tests before correcting it  because mistaking is easy in these kind of things.")
    if (bpp == 1) {
        return (data[start] & (0x80 >> shift)) != 0;
    } else if (bpp == 4) {
        if (shift == 0) {
            return (data[start] & 0xf0) >> 4;
        } else {
            return data[start] & 0x0f;
        }
    }
    return 0;
}


static inline int load_pointer(const char* file_name, uint8_t* data, uint8_t* mask, int* x, int* y)
{
    int rv = 0;

    try {
        if (access(file_name, F_OK)){
            LOG(LOG_WARNING, "pointer file [%s] does not exist\n", file_name);
            throw 1;
        }
        Stream stream(8192);
        int fd = open(file_name, O_RDONLY);
        if (fd < 1) {
            LOG(LOG_WARNING, "loading pointer from file [%s] failed\n", file_name);
            throw 1;
        }

        TODO("We should define some kind of transport object to read into the stream")

        int lg = read(fd, stream.data, 8192);
        if (!lg){
            throw 1;
        }
        close(fd);
        stream.end = stream.data + lg;

        TODO("the ways we do it now we have some risk of reading out of buffer (data that are not from file)")

        stream.in_skip_bytes(6);
        int w = stream.in_uint8();
        int h = stream.in_uint8();
        stream.in_skip_bytes(2);
        *x = stream.in_uint16_le();
        *y = stream.in_uint16_le();
        stream.in_skip_bytes(22);
        int bpp = stream.in_uint8();
        stream.in_skip_bytes(25);

        BGRPalette palette;
        if (w == 32 && h == 32) {
            if (bpp == 1) {
                memcpy(palette, stream.in_uint8p(8), 8);
                // read next 32x32 bytes
                for (int i = 0; i < 32; i++) {
                    for (int j = 0; j < 32; j++) {
                        int pixel = palette[get_pixel(stream.p, j, i, 32, 1)];
                        *data = pixel;
                        data++;
                        *data = pixel >> 8;
                        data++;
                        *data = pixel >> 16;
                        data++;
                    }
                }
                stream.in_skip_bytes(128);
            } else if (bpp == 4) {
                memcpy(palette, stream.in_uint8p(64), 64);
                for (int i = 0; i < 32; i++) {
                    for (int j = 0; j < 32; j++) {
            TODO(" probably bogus  we are in case bpp = 4 and we call get_pixel with 1 as bpp")
                        int pixel = palette[get_pixel(stream.p, j, i, 32, 1)];
                        *data = pixel;
                        data++;
                        *data = pixel >> 8;
                        data++;
                        *data = pixel >> 16;
                        data++;
                    }
                }
                stream.in_skip_bytes(512);
            }
            memcpy(mask, stream.p, 128); /* mask */
        }
    }
    catch(...){
        rv = 1;
    }
    return rv;
}

class Front : public FrontAPI {
public:
    RDPGraphicDevice * capture;
    GraphicsUpdatePDU * orders;
    Keymap2 keymap;
    ChannelList channel_list;
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
    Cache cache;

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

    Front(SocketTransport * trans, Random * gen, Inifile * ini) :
        FrontAPI(ini->globals.notimestamp, ini->globals.nomouse),
        capture(NULL),
        orders(NULL),
        up_and_running(0),
        share_id(65538),
        client_info(ini->globals.crypt_level, ini->globals.channel_code, ini->globals.bitmap_compression, ini->globals.bitmap_cache),
        packet_number(1),
        trans(trans),
        userid(0),
        order_level(0),
        ini(ini),
        verbose(this->ini?this->ini->globals.debug.front:0),
        font(SHARE_PATH "/" DEFAULT_FONT_NAME),
        cache(),
        state(CONNECTION_INITIATION),
        gen(gen)
    {
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
        this->mod_palette_setted = false;
        this->mod_bpp = 24;
        this->palette_sent = false;
        for (size_t i = 0; i < 6 ; i++){
            this->palette_memblt_sent[i] = false;
        }
        init_palette332(this->palette332);
    }

    void set_mod_bpp(uint8_t bpp)
    {
        this->mod_bpp = bpp;
    }

    int server_resize(int width, int height, int bpp)
    {
        if (this->client_info.width != width
        || this->client_info.height != height
        || this->client_info.bpp != bpp) {
            /* older client can't resize */
            if (client_info.build <= 419) {
                LOG(LOG_ERR, "Resizing is not available on older RDP clients");
                // resizing needed but not available
                return -1;
            }
            this->palette_sent = false;
            for (size_t i = 0; i < 6 ; i++){
                this->palette_memblt_sent[i] = false;
            }
            LOG(LOG_INFO, "// Resizing client to : %d x %d x %d\n", width, height, bpp);

            this->client_info.width = width;
            this->client_info.height = height;
            this->client_info.bpp = bpp;

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
            return 1;
        }
        // resizing not necessary
        return 0;
    }

    void server_set_pointer(int x, int y, uint8_t* data, uint8_t* mask)
    {
        int cache_idx = 0;
        switch (this->cache.add_pointer(data, mask, x, y, cache_idx)){
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
            TODO(" avoid passing parameters by reference to get results")
            switch (this->cache.add_glyph(font_item, f, c))
            {
                case Cache::GLYPH_ADDED_TO_CACHE:
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
            Rect(), // op
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
                const char * codec_id, const char * quality)
    {
        if (flag){
            this->stop_capture();
            this->capture = new Capture(width, height, 24, this->palette332,
                                           path, codec_id, quality);
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

        this->cache.reset(this->client_info);
    }

    void init_pointers()
    {
        struct pointer_item pointer_item;

        memset(&pointer_item, 0, sizeof(pointer_item));
        load_pointer(SHARE_PATH "/" CURSOR0,
            pointer_item.data,
            pointer_item.mask,
            &pointer_item.x,
            &pointer_item.y);

        this->cache.add_pointer_static(&pointer_item, 0);
        this->send_pointer(0,
                         pointer_item.data,
                         pointer_item.mask,
                         pointer_item.x,
                         pointer_item.y);

        memset(&pointer_item, 0, sizeof(pointer_item));
        load_pointer(SHARE_PATH "/" CURSOR1,
            pointer_item.data,
            pointer_item.mask,
            &pointer_item.x,
            &pointer_item.y);

        this->cache.add_pointer_static(&pointer_item, 1);

        this->send_pointer(1,
                 pointer_item.data,
                 pointer_item.mask,
                 pointer_item.x,
                 pointer_item.y);
    }

    virtual void begin_update()
    {
        if (this->verbose & 4){
            LOG(LOG_INFO, "Front::begin_update()");
        }
        this->order_level++;
    }

    virtual void end_update()
    {
        if (this->verbose & 4){
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
        Stream stream(32768);
        X224Out tpdu(X224Packet::DT_TPDU, stream);

        stream.out_uint8((MCS_DPUM << 2) | 1);
        stream.out_uint8(0x80);

        tpdu.end();
        tpdu.send(this->trans);
    }

    void set_console_session(bool b)
    {
        if (this->verbose){
            LOG(LOG_INFO, "Front::set_console_session(%u)", b);
        }
        this->client_info.console_session = b;
    }

    virtual const ChannelList & get_channel_list(void) const
    {
        return this->channel_list;
    }

    virtual void send_to_channel(
        const McsChannelItem & channel,
        uint8_t* data,
        size_t length,
        size_t chunk_size,
        int flags)
    {
        if (this->verbose){
            LOG(LOG_INFO, "Front::send_to_channel(channel, data=%p, length=%u, chunk_size=%u, flags=%x)", data, length, chunk_size, flags);
        }
        Stream stream(65536);
        X224Out tpdu(X224Packet::DT_TPDU, stream);
        McsOut sdin_out(stream, MCS_SDIN, this->userid, channel.chanid);
        uint32_t sec_flags = this->client_info.crypt_level?SEC_ENCRYPT:0;
        SecOut sec_out(stream, sec_flags, this->encrypt);

        stream.out_uint32_le(length);
        if (channel.flags & CHANNEL_OPTION_SHOW_PROTOCOL) {
            flags |= CHANNEL_FLAG_SHOW_PROTOCOL;
        }
        stream.out_uint32_le(flags);
        stream.out_copy_bytes(data, chunk_size);

        sec_out.end();
        sdin_out.end();
        tpdu.end();
        tpdu.send(this->trans);
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

            const BGRPalette & palette =
                (this->mod_bpp == 8)?this->memblt_mod_palette:this->palette332;

            if (this->verbose){
                LOG(LOG_INFO, "Front::send_global_palette()");
            }
            Stream stream(32768);
            X224Out tpdu(X224Packet::DT_TPDU, stream);
            McsOut sdin_out(stream, MCS_SDIN, this->userid, MCS_GLOBAL_CHANNEL);
            uint32_t sec_flags = this->client_info.crypt_level?SEC_ENCRYPT:0;
            SecOut sec_out(stream, sec_flags, this->encrypt);
            ShareControlOut rdp_control_out(stream, PDUTYPE_DATAPDU, this->userid + MCS_USERCHANNEL_BASE);
            ShareDataOut rdp_data_out(stream, PDUTYPE2_UPDATE, this->share_id, RDP::STREAM_MED);

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

            rdp_data_out.end();
            rdp_control_out.end();
            sec_out.end();
            sdin_out.end();
            tpdu.end();
            tpdu.send(this->trans);

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
        Stream stream(32768);
        X224Out tpdu(X224Packet::DT_TPDU, stream);
        McsOut sdin_out(stream, MCS_SDIN, this->userid, MCS_GLOBAL_CHANNEL);
        uint32_t sec_flags = this->client_info.crypt_level?SEC_ENCRYPT:0;
        SecOut sec_out(stream, sec_flags, this->encrypt);
        ShareControlOut rdp_control_out(stream, PDUTYPE_DATAPDU, this->userid + MCS_USERCHANNEL_BASE);
        ShareDataOut rdp_data_out(stream, PDUTYPE2_POINTER, this->share_id, RDP::STREAM_MED);

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

        rdp_data_out.end();
        rdp_control_out.end();
        sec_out.end();
        sdin_out.end();
        tpdu.end();
        tpdu.send(this->trans);

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
        Stream stream(32768);
        X224Out tpdu(X224Packet::DT_TPDU, stream);
        McsOut sdin_out(stream, MCS_SDIN, this->userid, MCS_GLOBAL_CHANNEL);
        uint32_t sec_flags = this->client_info.crypt_level?SEC_ENCRYPT:0;
        SecOut sec_out(stream, sec_flags, this->encrypt);
        ShareControlOut rdp_control_out(stream, PDUTYPE_DATAPDU, this->userid + MCS_USERCHANNEL_BASE);
        ShareDataOut rdp_data_out(stream, PDUTYPE2_POINTER, this->share_id, RDP::STREAM_MED);

        stream.out_uint16_le(RDP_POINTER_CACHED);
        stream.out_uint16_le(0); /* pad */
        stream.out_uint16_le(cache_idx);

        rdp_data_out.end();
        rdp_control_out.end();
        sec_out.end();
        sdin_out.end();
        tpdu.end();
        tpdu.send(this->trans);
        if (this->verbose){
            LOG(LOG_INFO, "Front::set_pointer done");
        }

    }


//   2.2.1.5 Client MCS Erect Domain Request PDU
//   -------------------------------------------
//   The MCS Erect Domain Request PDU is an RDP Connection Sequence PDU sent
//   from client to server during the Channel Connection phase (see section
//   1.3.1.1). It is sent after receiving the MCS Connect Response PDU (section
//   2.2.1.4).

//   tpktHeader (4 bytes): A TPKT Header, as specified in [T123] section 8.

//   x224Data (3 bytes): An X.224 Class 0 Data TPDU, as specified in [X224]
//      section 13.7.

// See description of tpktHeader and x224 Data TPDU in cheat sheet

//   mcsEDrq (5 bytes): PER-encoded MCS Domain PDU which encapsulates an MCS
//      Erect Domain Request structure, as specified in [T125] (the ASN.1
//      structure definitions are given in [T125] section 7, parts 3 and 10).

// 2.2.1.6 Client MCS Attach User Request PDU
// ------------------------------------------
// The MCS Attach User Request PDU is an RDP Connection Sequence PDU
// sent from client to server during the Channel Connection phase (see
// section 1.3.1.1) to request a user channel ID. It is sent after
// transmitting the MCS Erect Domain Request PDU (section 2.2.1.5).

// tpktHeader (4 bytes): A TPKT Header, as specified in [T123] section 8.

// x224Data (3 bytes): An X.224 Class 0 Data TPDU, as specified in
//   [X224] section 13.7.

// See description of tpktHeader and x224 Data TPDU in cheat sheet

// mcsAUrq (1 byte): PER-encoded MCS Domain PDU which encapsulates an
//  MCS Attach User Request structure, as specified in [T125] (the ASN.1
//  structure definitions are given in [T125] section 7, parts 5 and 10).

// AttachUserRequest ::= [APPLICATION 10] IMPLICIT SEQUENCE
// {
// }

// 11.17 AttachUserRequest
// -----------------------

// AttachUserRequest is generated by an MCS-ATTACH-USER request. It rises to the
// top MCS provider, which returns an AttachUserConfirm reply. If the domain
// limit on number of user ids allows, a new user id is generated.

// AttachUserRequest contains no information other than its MCSPDU type. The
// domain to which the user attaches is determined by the MCS connection
// conveying the MCSPDU. The only initial characteristic of the user id
// generated is its uniqueness. An MCS provider shall make a record of each
// unanswered AttachUserRequest received and by which MCS connection it arrived,
// so that a replying AttachUserConfirm can be routed back to the same source.
// To distribute replies fairly, each provider should maintain a first-in,
// first-out queue for this purpose.

    void recv_mcs_erect_domain_and_attach_user_request_pdu(Transport * trans, uint16_t & userid)
    {
        TODO(" this code could lead to some problem if both MCS are combined in the same TPDU  we should manage this case")
        {
            Stream stream(32768);
            X224In in(trans, stream);
            uint8_t opcode = stream.in_uint8();
            if ((opcode >> 2) != MCS_EDRQ) {
                throw Error(ERR_MCS_RECV_EDQR_APPID_NOT_EDRQ);
            }
            stream.in_skip_bytes(2);
            stream.in_skip_bytes(2);
            if (opcode & 2) {
                userid = stream.in_uint16_be();
            }
            in.end();
        }

        {
            Stream stream(32768);
            X224In in(trans, stream);
            uint8_t opcode = stream.in_uint8();
            if ((opcode >> 2) != MCS_AURQ) {
                throw Error(ERR_MCS_RECV_AURQ_APPID_NOT_AURQ);
            }
            if (opcode & 2) {
                userid = stream.in_uint16_be();
            }
            in.end();
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

    void send_mcs_connect_response_pdu_with_gcc_conference_create_response(
                                        Transport * trans,
                                        ClientInfo * client_info,
                                        const ChannelList & channel_list,
                                        uint8_t (&server_random)[32],
                                        int rc4_key_size,
                                        uint8_t (&pub_mod)[512],
                                        uint8_t (&pri_exp)[512]
                                    ) throw(Error)
    {
        Stream stream(32768);

        // TPKT Header (length = 337 bytes)
        // X.224 Data TPDU
        X224Out tpdu(X224Packet::DT_TPDU, stream);

        // BER: Application-Defined Type = APPLICATION 102 = Connect-Response
        stream.out_uint16_be(BER_TAG_MCS_CONNECT_RESPONSE);
        uint32_t offset_len_mcs_connect_response = stream.get_offset(0);
        // BER: Type Length
        stream.out_ber_len_uint16(0); // filled later, 3 bytes

        // Connect-Response::result = rt-successful (0)
        // The first byte (0x0a) is the ASN.1 BER encoded Enumerated type. The
        // length of the value is given by the second byte (1 byte), and the
        // actual value is 0 (rt-successful).
        stream.out_uint8(BER_TAG_RESULT);
        stream.out_ber_len_uint7(1);
        stream.out_uint8(0);

        // Connect-Response::calledConnectId = 0
        stream.out_uint8(BER_TAG_INTEGER);
        stream.out_ber_len_uint7(1);
        stream.out_uint8(0);

        // Connect-Response::domainParameters (26 bytes)
        stream.out_uint8(BER_TAG_MCS_DOMAIN_PARAMS);
        stream.out_ber_len_uint7(26);
        // DomainParameters::maxChannelIds = 34
        stream.out_ber_int8(22);
        // DomainParameters::maxUserIds = 3
        stream.out_ber_int8(3);
        // DomainParameters::maximumTokenIds = 0
        stream.out_ber_int8(0);
        // DomainParameters::numPriorities = 1
        stream.out_ber_int8(1);
        // DomainParameters::minThroughput = 0
        stream.out_ber_int8(0);
        // DomainParameters::maxHeight = 1
        stream.out_ber_int8(1);
        // DomainParameters::maxMCSPDUsize = 65528
        stream.out_ber_int24(0xfff8);
        // DomainParameters::protocolVersion = 2
        stream.out_ber_int8(2);

        // Connect-Response::userData (287 bytes)
        stream.out_uint8(BER_TAG_OCTET_STRING);
        uint32_t offset_len_mcs_data = stream.get_offset(0);
        stream.out_ber_len_uint16(0); // filled later, 3 bytes


        // GCC Conference Create Response
        // ------------------------------

        // ConferenceCreateResponse Parameters
        // -----------------------------------

        // Generic definitions used in parameter descriptions:

        // simpleTextFirstCharacter UniversalString ::= {0, 0, 0, 0}

        // simpleTextLastCharacter UniversalString ::= {0, 0, 0, 255}

        // SimpleTextString ::=  BMPString (SIZE (0..255)) (FROM (simpleTextFirstCharacter..simpleTextLastCharacter))

        // TextString ::= BMPString (SIZE (0..255)) -- Basic Multilingual Plane of ISO/IEC 10646-1 (Unicode)

        // SimpleNumericString ::= NumericString (SIZE (1..255)) (FROM ("0123456789"))

        // DynamicChannelID ::= INTEGER (1001..65535) -- Those created and deleted by MCS

        // UserID ::= DynamicChannelID

        // H221NonStandardIdentifier ::= OCTET STRING (SIZE (4..255))
        //      -- First four octets shall be country code and
        //      -- Manufacturer code, assigned as specified in
        //      -- Annex A/H.221 for NS-cap and NS-comm

        // Key ::= CHOICE   -- Identifier of a standard or non-standard object
        // {
        //      object              OBJECT IDENTIFIER,
        //      h221NonStandard     H221NonStandardIdentifier
        // }

        // UserData ::= SET OF SEQUENCE
        // {
        //      key     Key,
        //      value   OCTET STRING OPTIONAL
        // }

        // ConferenceCreateResponse ::= SEQUENCE
        // {    -- MCS-Connect-Provider response user data
        //      nodeID              UserID, -- Node ID of the sending node
        //      tag                 INTEGER,
        //      result              ENUMERATED
        //      {
        //          success                         (0),
        //          userRejected                    (1),
        //          resourcesNotAvailable           (2),
        //          rejectedForSymmetryBreaking     (3),
        //          lockedConferenceNotSupported    (4),
        //          ...
        //      },
        //      userData            UserData OPTIONAL,
        //      ...
        //}


        // User Data                 : Optional
        // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

        // User Data: Optional user data which may be used for functions outside
        // the scope of this Recommendation such as authentication, billing,
        // etc.

        // Result                    : Mandatory
        // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

        // An indication of whether the request was accepted or rejected, and if
        // rejected, the reason why. It contains one of a list of possible
        // results: successful, user rejected, resources not available, rejected
        // for symmetry-breaking, locked conference not supported, Conference
        // Name and Conference Name Modifier already exist, domain parameters
        // unacceptable, domain not hierarchical, lower-layer initiated
        // disconnect, unspecified failure to connect. A negative result in the
        // GCC-Conference-Create confirm does not imply that the physical
        // connection to the node to which the connection was being attempted
        // is disconnected.

        // The ConferenceCreateResponse PDU is shown in Table 8-4. The Node ID
        // parameter, which is the User ID assigned by MCS in response to the
        // MCS-Attach-User request issued by the GCC Provider, shall be supplied
        // by the GCC Provider sourcing this PDU. The Tag parameter is assigned
        // by the source GCC Provider to be locally unique. It is used to
        // identify the returned UserIDIndication PDU. The Result parameter
        // includes GCC-specific failure information sourced directly from
        // the Result parameter in the GCC-Conference-Create response primitive.
        // If the Result parameter is anything except successful, the Result
        // parameter in the MCS-Connect-Provider response is set to
        // user-rejected.

        //            Table 8-4 – ConferenceCreateResponse GCCPDU
        // +------------------+------------------+--------------------------+
        // | Content          |     Source       |         Sink             |
        // +==================+==================+==========================+
        // | Node ID          | Top GCC Provider | Destination GCC Provider |
        // +------------------+------------------+--------------------------+
        // | Tag              | Top GCC Provider | Destination GCC Provider |
        // +------------------+------------------+--------------------------+
        // | Result           | Response         | Confirm                  |
        // +------------------+------------------+--------------------------+
        // | User Data (opt.) | Response         | Confirm                  |
        // +------------------+------------------+--------------------------+

        //PER encoded (ALIGNED variant of BASIC-PER) GCC Connection Data (ConnectData):
        // 00 05 00
        // 14 7c 00 01
        // 2a
        // 14 76 0a 01 01 00 01 c0 00 4d 63 44 6e
        // 81 08


        // 00 05 -> Key::object length = 5 bytes
        // 00 14 7c 00 01 -> Key::object = { 0 0 20 124 0 1 }
        stream.out_uint16_be(5);
        stream.out_copy_bytes("\x00\x14\x7c\x00\x01", 5);


        // 2a -> ConnectData::connectPDU length = 42 bytes
        // This length MUST be ignored by the client.
        stream.out_uint8(0x2a);

        // PER encoded (ALIGNED variant of BASIC-PER) GCC Conference Create Response
        // PDU:
        // 14 76 0a 01 01 00 01 c0 00 00 4d 63 44 6e 81 08

        // 0x14:
        // 0 - extension bit (ConnectGCCPDU)
        // 0 - --\ ...
        // 0 -   | CHOICE: From ConnectGCCPDU select conferenceCreateResponse (1)
        // 1 - --/ of type ConferenceCreateResponse
        // 0 - extension bit (ConferenceCreateResponse)
        // 1 - ConferenceCreateResponse::userData present
        // 0 - padding
        // 0 - padding
        stream.out_uint8(0x10 | 4);

        // ConferenceCreateResponse::nodeID
        //  = 0x760a + 1001 = 30218 + 1001 = 31219
        //  (minimum for UserID is 1001)
        stream.out_uint16_le(0x760a);

        // ConferenceCreateResponse::tag length = 1 byte
        stream.out_uint8(1);

        // ConferenceCreateResponse::tag = 1
        stream.out_uint8(1);

        // 0x00:
        // 0 - extension bit (Result)
        // 0 - --\ ...
        // 0 -   | ConferenceCreateResponse::result = success (0)
        // 0 - --/
        // 0 - padding
        // 0 - padding
        // 0 - padding
        // 0 - padding
        stream.out_uint8(0);

        // number of UserData sets = 1
        stream.out_uint8(1);

        // 0xc0:
        // 1 - UserData::value present
        // 1 - CHOICE: From Key select h221NonStandard (1)
        //               of type H221NonStandardIdentifier
        // 0 - padding
        // 0 - padding
        // 0 - padding
        // 0 - padding
        // 0 - padding
        // 0 - padding
        stream.out_uint8(0xc0);

        // h221NonStandard length = 0 + 4 = 4 octets
        //   (minimum for H221NonStandardIdentifier is 4)
        stream.out_uint8(0);

        // h221NonStandard (server-to-client H.221 key) = "McDn"
        stream.out_copy_bytes("McDn", 4);

//        uint16_t padding = channel_list.size() & 1;
//        uint16_t srv_channel_size = 8 + (channel_list.size() + padding) * 2;
//        stream.out_2BUE(8 + srv_channel_size + 236 + 4); // len


        uint32_t offset_user_data_len = stream.get_offset(0);
        stream.out_uint16_be(0);

        bool use_rdp5 = 1;
        out_mcs_data_sc_core(stream, use_rdp5);
        out_mcs_data_sc_net(stream, channel_list);
        front_out_gcc_conference_user_data_sc_sec1(stream, client_info->crypt_level, server_random, rc4_key_size, pub_mod, pri_exp, this->gen);

        TODO(" create a function in stream that sets differed ber_len_offsets (or other len_offset)")

        // set user_data_len (TWO_BYTE_UNSIGNED_ENCODING)
        stream.set_out_uint16_be(0x8000 | (stream.get_offset(offset_user_data_len + 2)), offset_user_data_len);
        // set mcs_data len, BER_TAG_OCTET_STRING (some kind of BLOB)
        stream.set_out_ber_len_uint16(stream.get_offset(offset_len_mcs_data + 3), offset_len_mcs_data);
        // set BER_TAG_MCS_CONNECT_RESPONSE len
        stream.set_out_ber_len_uint16(stream.get_offset(offset_len_mcs_connect_response + 3), offset_len_mcs_connect_response);

        tpdu.end();
        tpdu.send(trans);
    }


// 2.2.1.3 Client MCS Connect Initial PDU with GCC Conference Create Request
// =========================================================================

// The MCS Connect Initial PDU is an RDP Connection Sequence PDU sent from
// client to server during the Basic Settings Exchange phase (see section
// 1.3.1.1). It is sent after receiving the X.224 Connection Confirm PDU
// (section 2.2.1.2). The MCS Connect Initial PDU encapsulates a GCC Conference
// Create Request, which encapsulates concatenated blocks of settings data. A
// basic high-level overview of the nested structure for the Client MCS Connect
// Initial PDU is illustrated in section 1.3.1.1, in the figure specifying MCS
// Connect Initial PDU. Note that the order of the settings data blocks is
// allowed to vary from that shown in the previously mentioned figure and the
// message syntax layout that follows. This is possible because each data block
// is identified by a User Data Header structure (section 2.2.1.3.1).

// tpktHeader (4 bytes): A TPKT Header, as specified in [T123] section 8.

// x224Data (3 bytes): An X.224 Class 0 Data TPDU, as specified in [X224]
//   section 13.7.

// mcsCi (variable): Variable-length BER-encoded MCS Connect Initial structure
//   (using definite-length encoding) as described in [T125] (the ASN.1
//   structure definition is detailed in [T125] section 7, part 2). The userData
//   field of the MCS Connect Initial encapsulates the GCC Conference Create
//   Request data (contained in the gccCCrq and subsequent fields). The maximum
//   allowed size of this user data is 1024 bytes, which implies that the
//   combined size of the gccCCrq and subsequent fields MUST be less than 1024
//   bytes.

// gccCCrq (variable): Variable-length Packed Encoding Rule encoded
//   (PER-encoded) GCC Connect Data structure, which encapsulates a Connect GCC
//   PDU that contains a GCC Conference Create Request structure as described in
//   [T124] (the ASN.1 structure definitions are detailed in [T124] section 8.7)
//   appended as user data to the MCS Connect Initial (using the format
//   described in [T124] sections 9.5 and 9.6). The userData field of the GCC
//   Conference Create Request contains one user data set consisting of
//   concatenated client data blocks.

// clientCoreData (216 bytes): Client Core Data structure (section 2.2.1.3.2).

// clientSecurityData (12 bytes): Client Security Data structure (section
//   2.2.1.3.3).

// clientNetworkData (variable): Optional and variable-length Client Network
//   Data structure (section 2.2.1.3.4).

// clientClusterData (12 bytes): Optional Client Cluster Data structure (section
//   2.2.1.3.5).

// clientMonitorData (variable): Optional Client Monitor Data structure (section
//   2.2.1.3.6). This field MUST NOT be included if the server does not
//   advertise support for extended client data blocks by using the
//   EXTENDED_CLIENT_DATA_SUPPORTED flag (0x00000001) as described in section
//   2.2.1.2.1.



    void recv_mcs_connect_initial_pdu_with_gcc_conference_create_request(
                    Transport * trans,
                    ClientInfo * client_info,
                    ChannelList & channel_list)
    {
        Stream stream(32768);
        X224In(trans, stream);

        if (stream.in_uint16_be() != BER_TAG_MCS_CONNECT_INITIAL) {
            throw Error(ERR_MCS_BER_HEADER_UNEXPECTED_TAG);
        }
        int len = stream.in_ber_len();
        if (stream.in_uint8() != BER_TAG_OCTET_STRING) {
            throw Error(ERR_MCS_BER_HEADER_UNEXPECTED_TAG);
        }
        len = stream.in_ber_len();
        stream.in_skip_bytes(len);

        if (stream.in_uint8() != BER_TAG_OCTET_STRING) {
            throw Error(ERR_MCS_BER_HEADER_UNEXPECTED_TAG);
        }
        len = stream.in_ber_len();
        stream.in_skip_bytes(len);
        if (stream.in_uint8() != BER_TAG_BOOLEAN) {
            throw Error(ERR_MCS_BER_HEADER_UNEXPECTED_TAG);
        }
        len = stream.in_ber_len();
        stream.in_skip_bytes(len);

        if (stream.in_uint8() != BER_TAG_MCS_DOMAIN_PARAMS) {
            throw Error(ERR_MCS_BER_HEADER_UNEXPECTED_TAG);
        }
        len = stream.in_ber_len();
        stream.in_skip_bytes(len);

        if (stream.in_uint8() != BER_TAG_MCS_DOMAIN_PARAMS) {
            throw Error(ERR_MCS_BER_HEADER_UNEXPECTED_TAG);
        }
        len = stream.in_ber_len();
        stream.in_skip_bytes(len);

        if (stream.in_uint8() != BER_TAG_MCS_DOMAIN_PARAMS) {
            throw Error(ERR_MCS_BER_HEADER_UNEXPECTED_TAG);
        }
        len = stream.in_ber_len();
        stream.in_skip_bytes(len);

        if (stream.in_uint8() != BER_TAG_OCTET_STRING) {
            throw Error(ERR_MCS_BER_HEADER_UNEXPECTED_TAG);
        }
        len = stream.in_ber_len();

        stream.in_skip_bytes(23);

    // 2.2.1.3.1 User Data Header (TS_UD_HEADER)
    // =========================================

    // type (2 bytes): A 16-bit, unsigned integer. The type of the data
    //                 block that this header precedes.

    // +-------------------+-------------------------------------------------------+
    // | CS_CORE 0xC001    | The data block that follows contains Client Core      |
    // |                   | Data (section 2.2.1.3.2).                             |
    // +-------------------+-------------------------------------------------------+
    // | CS_SECURITY 0xC002| The data block that follows contains Client           |
    // |                   | Security Data (section 2.2.1.3.3).                    |
    // +-------------------+-------------------------------------------------------+
    // | CS_NET 0xC003     | The data block that follows contains Client Network   |
    // |                   | Data (section 2.2.1.3.4).                             |
    // +-------------------+-------------------------------------------------------+
    // | CS_CLUSTER 0xC004 | The data block that follows contains Client Cluster   |
    // |                   | Data (section 2.2.1.3.5).                             |
    // +-------------------+-------------------------------------------------------+
    // | CS_MONITOR 0xC005 | The data block that follows contains Client           |
    // |                   | Monitor Data (section 2.2.1.3.6).                     |
    // +-------------------+-------------------------------------------------------+
    // | SC_CORE 0x0C01    | The data block that follows contains Server Core      |
    // |                   | Data (section 2.2.1.4.2)                              |
    // +-------------------+-------------------------------------------------------+
    // | SC_SECURITY 0x0C02| The data block that follows contains Server           |
    // |                   | Security Data (section 2.2.1.4.3).                    |
    // +-------------------+-------------------------------------------------------+
    // | SC_NET 0x0C03     | The data block that follows contains Server Network   |
    // |                   | Data (section 2.2.1.4.4)                              |
    // +-------------------+-------------------------------------------------------+

    // length (2 bytes): A 16-bit, unsigned integer. The size in bytes of the data
    //   block, including this header.

        while (stream.check_rem(4)) {
            uint8_t * current_header = stream.p;
            uint16_t tag = stream.in_uint16_le();
            uint16_t length = stream.in_uint16_le();
            if (length < 4 || !stream.check_rem(length - 4)) {
                LOG(LOG_ERR,
                    "error reading block tag %d size %d\n",
                    tag, length);
                break;
            }

            switch (tag){
                case CS_CORE:
                    TODO(" we should check length to call the two variants of core_data (or begin by reading the common part then the extended part)")
                    parse_mcs_data_cs_core(stream, client_info);
                break;
                case CS_SECURITY:
                    parse_mcs_data_cs_security(stream);
                break;
                case CS_NET:
                    parse_mcs_data_cs_net(stream, client_info, channel_list);
                break;
                case CS_CLUSTER:
                    parse_mcs_data_cs_cluster(stream, client_info);
                break;
                case CS_MONITOR:
                    parse_mcs_data_cs_monitor(stream);
                break;
                case SC_SECURITY:
    //                parse_mcs_data_sc_security(stream);
                break;
                default:
                    LOG(LOG_INFO, "Unexpected data block tag %x\n", tag);
                break;
            }
            stream.p = current_header + length;
        }
    }


// 2.2.1.7 Server MCS Attach User Confirm PDU
// ------------------------------------------
// The MCS Attach User Confirm PDU is an RDP Connection Sequence
// PDU sent from server to client during the Channel Connection
// phase (see section 1.3.1.1). It is sent as a response to the MCS
// Attach User Request PDU (section 2.2.1.6).

// tpktHeader (4 bytes): A TPKT Header, as specified in [T123]
//   section 8.

// x224Data (3 bytes): An X.224 Class 0 Data TPDU, as specified in
//   section [X224] 13.7.

// mcsAUcf (4 bytes): PER-encoded MCS Domain PDU which encapsulates
//   an MCS Attach User Confirm structure, as specified in [T125]
//   (the ASN.1 structure definitions are given in [T125] section 7,
// parts 5 and 10).

// AttachUserConfirm ::= [APPLICATION 11] IMPLICIT SEQUENCE
// {
//     result       Result,
//     initiator    UserId OPTIONAL
// }

// 11.18 AttachUserConfirm
// -----------------------

// AttachUserConfirm is generated at the top MCS provider upon receipt of
// AttachUserRequest. Routed back to the requesting provider, it generates an
//  MCS-ATTACH-USER confirm.

//      Table 11-18/T.125 – AttachUserConfirm MCSPDU
// +----------------------+-----------------+------------+
// |     Contents         |      Source     |    Sink    |
// +----------------------+-----------------+------------+
// | Result               | Top provider    |  Confirm   |
// +----------------------+-----------------+------------+
// | Initiator (optional) | Top provider    |  Confirm   |
// +----------------------+-----------------+------------+

// AttachUserConfirm contains a user id if and only if the result is successful.
// Providers that receive a successful AttachUserConfirm shall enter the user id
// into their information base. MCS providers shall route AttachUserConfirm to
// the source of an antecedent AttachUserRequest, using the knowledge that
// there is a one-to-one reply. A provider that transmits AttachUserConfirm
// shall note to which downward MCS connection the new user id is thereby
// assigned, so that it may validate the user id when it arises later in other
// requests.

    void send_mcs_attach_user_confirm_pdu(Transport * trans, uint16_t userid)
    {
        Stream stream(32768);
        X224Out tpdu(X224Packet::DT_TPDU, stream);
        stream.out_uint8(((MCS_AUCF << 2) | 2));
        stream.out_uint8(0);
        stream.out_uint16_be(userid);
        tpdu.end();
        tpdu.send(trans);
    }


// 2.2.1.9 Server MCS Channel Join Confirm PDU
// -------------------------------------------
// The MCS Channel Join Confirm PDU is an RDP Connection Sequence
// PDU sent from server to client during the Channel Connection
// phase (see section 1.3.1.1). It is sent as a response to the MCS
// Channel Join Request PDU (section 2.2.1.8).

// tpktHeader (4 bytes): A TPKT Header, as specified in [T123]
//   section 8.

// x224Data (3 bytes): An X.224 Class 0 Data TPDU, as specified in
//  [X224] section 13.7.

// mcsCJcf (8 bytes): PER-encoded MCS Domain PDU which encapsulates
//  an MCS Channel Join Confirm PDU structure, as specified in
//  [T125] (the ASN.1 structure definitions are given in [T125]
//  section 7, parts 6 and 10).

// ChannelJoinConfirm ::= [APPLICATION 15] IMPLICIT SEQUENCE
// {
//   result Result,
//   initiator UserId,
//   requested ChannelId, -- may be zero
//   channelId ChannelId OPTIONAL
// }

// 11.22 ChannelJoinConfirm
// ------------------------

// ChannelJoinConfirm is generated at a higher MCS provider upon receipt of
// ChannelJoinRequest. Routed back to the requesting provider, it generates an
// MCS-CHANNEL-JOIN confirm.

// Table 11-22/T.125 – ChannelJoinConfirm MCSPDU
// +-----------------------+------------------------+--------------------------+
// | Contents              |       Source           |         Sink             |
// +-----------------------+------------------------+--------------------------+
// | Result                |   Higher provider      |        Confirm           |
// +-----------------------+------------------------+--------------------------+
// | Initiator             |   Higher provider      |        MCSPDU routing    |
// +-----------------------+------------------------+--------------------------+
// | Requested             |   Higher provider      |        Confirm           |
// +-----------------------+------------------------+--------------------------+
// | Channel Id (optional) |   Higher provider      |        Confirm           |
// +-----------------------+------------------------+--------------------------+


// ChannelJoinConfirm contains a joined channel id if and only if the result is
// successful.


// The channel id requested is the same as in ChannelJoinRequest. This helps
// the initiating attachment relate MCS-CHANNEL-JOIN confirm to an antecedent
// request. Since ChannelJoinRequest need not rise to the top provider,
// confirms may occur out of order.

// If the result is successful, ChannelJoinConfirm joins the receiving MCS
// provider to the specified channel. Thereafter, higher providers shall route
// to it any data that users send over the channel. A provider shall remain
// joined to a channel as long as any of its attachments or subordinate
// providers does. To leave the channel, a provider shall generate
// ChannelLeaveRequest.

// Providers that receive a successful ChannelJoinConfirm shall enter the
// channel id into their information base. If not already there, the channel id
// shall be given type static or assigned, depending on its range.

// ChannelJoinConfirm shall be forwarded in the direction of the initiating user
// id. If the user id is unreachable because an MCS connection no longer exists,
// the provider shall decide whether it has reason to remain joined to the
// channel. If not, it shall generate ChannelLeaveRequest.

    void send_mcs_channel_join_confirm_pdu(Transport * trans, uint16_t userid, uint16_t chanid)
    {
        Stream stream(32768);
        X224Out tpdu(X224Packet::DT_TPDU, stream);
        stream.out_uint8((MCS_CJCF << 2) | 2);
        stream.out_uint8(0);
        stream.out_uint16_be(userid);
        stream.out_uint16_be(chanid);
        TODO("this should be sent only if different from requested chan_id")
        stream.out_uint16_be(chanid);
        tpdu.end();
        tpdu.send(trans);
    }


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


// 11.21 ChannelJoinRequest
// ------------------------

// ChannelJoinRequest is generated by an MCS-CHANNEL-JOIN request. If valid, it
// rises until it reaches an MCS provider with enough information to generate a
// ChannelJoinConfirm reply. This may be the top MCS provider.

// Table 11-21/T.125 – ChannelJoinRequest MCSPDU
// +-----------------+-------------------------------+------------------------+
// | Contents        |           Source              |           Sink         |
// +-----------------+-------------------------------+------------------------+
// | Initiator       |      Requesting Provider      |       Higher provider  |
// +-----------------+-------------------------------+------------------------+
// | Channel Id      |      Request                  |       Higher provider  |
// +-----------------+-------------------------------+------------------------+


// The user id of the initiating MCS attachment is supplied by the MCS provider
// that receives the primitive request. Providers that receive
// ChannelJoinRequest subsequently shall validate the user id to ensure that it
// is legitimately assigned to the subtree of origin. If the user id is invalid,
// the MCSPDU shall be ignored.

// NOTE – This allows for the possibility that ChannelJoinRequest may be racing
// upward against a purge of the initiating user id flowing down. A provider
// that receives PurgeChannelsIndication first might receive a
// ChannelJoinRequest soon thereafter that contains an invalid user id. This is
// a normal occurrence and is not cause for rejecting the MCSPDU.

// ChannelJoinRequest may rise to an MCS provider that has the requested channel
// id in its information base. Any such provider, being consistent with the top
// MCS provider, will agree whether the request should succeed. If the request
// should fail, the provider shall generate an unsuccessful ChannelJoinConfirm.
// If it should succeed and the provider is already joined to the same channel,
// the provider shall generate a successful ChannelJoinConfirm. In these two
// cases, MCS-CHANNEL-JOIN completes without necessarily visiting the top MCS
// provider. Otherwise, if the request should succeed but the channel is not yet
// joined, a provider shall forward ChannelJoinRequest upward.

// If ChannelJoinRequest rises to the top MCS provider, the channel id
// requested may be zero, which is in no information base because it is an
// invalid id. If the domain limit on the number of channels in use allows,
// a new assigned channel id shall be generated and returned in a successful
// ChannelJoinConfirm. If the channel id requested is in the static range and
// the domain limit on the number of channels in use allows, the channel id
// shall be entered into the information base and shall likewise be returned
// in a successful ChannelJoinConfirm.

// Otherwise, the request will succeed only if the channel id is already in the
// information base of the top MCS provider. A user id channel can only be
// joined by the same user. A private channel id can be joined only by users
// previously admitted by its manager. An assigned channel id can be joined
// by any user.

    void recv_mcs_channel_join_request_pdu(Transport * trans, uint16_t & userid, uint16_t & chanid){
        Stream stream(32768);
        // read tpktHeader (4 bytes = 3 0 len)
        // TPDU class 0    (3 bytes = LI F0 PDU_DT)
        X224In in(trans, stream);

        uint8_t opcode = stream.in_uint8();
        if ((opcode >> 2) != MCS_CJRQ) {
            LOG(LOG_INFO, "unexpected opcode = %u", opcode);
            throw Error(ERR_MCS_RECV_CJRQ_APPID_NOT_CJRQ);
        }
        userid = stream.in_uint16_be();
        chanid = stream.in_uint16_be();

        if (opcode & 2) {
            stream.in_skip_bytes(2);
        }

        in.end();
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
                Stream in(8192);
                X224In crtpdu(this->trans, in);
                if (crtpdu.tpdu_hdr.code != X224Packet::CR_TPDU) {
                    LOG(LOG_INFO, "recv x224 connection request PDU failed code=%u", crtpdu.tpdu_hdr.code);
                    throw Error(ERR_ISO_INCOMING_CODE_NOT_PDU_CR);
                }
                crtpdu.end();
            }

            if (this->verbose){
                LOG(LOG_INFO, "Front::incoming::sending x224 connection confirm PDU");
            }
            {
                Stream out(128);
                X224Out cctpdu(X224Packet::CC_TPDU, out);
                cctpdu.end();
                cctpdu.send(this->trans);
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

            this->recv_mcs_connect_initial_pdu_with_gcc_conference_create_request(
                this->trans,
                &this->client_info,
                this->channel_list);

            this->send_mcs_connect_response_pdu_with_gcc_conference_create_response(
                this->trans,
                &this->client_info,
                this->channel_list,
                this->server_random,
                this->encrypt.rc4_key_size,
                this->pub_mod,
                this->pri_exp);

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
                LOG(LOG_INFO, "Front::incoming::recv_mcs_erect_domain_and_attach_user_request_pdu : user_id=%u", this->userid);
            }
            this->recv_mcs_erect_domain_and_attach_user_request_pdu(this->trans, this->userid);

            if (this->verbose){
                LOG(LOG_INFO, "Front::incoming::send_mcs_attach_user_confirm_pdu : user_id=%u", this->userid);
            }
            this->send_mcs_attach_user_confirm_pdu(this->trans, this->userid);

            {
                uint16_t tmp_userid;
                uint16_t tmp_chanid;
                recv_mcs_channel_join_request_pdu(this->trans, tmp_userid, tmp_chanid);
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
                this->send_mcs_channel_join_confirm_pdu(this->trans, this->userid, tmp_chanid);
            }

            {
                uint16_t tmp_userid;
                uint16_t tmp_chanid;
                this->recv_mcs_channel_join_request_pdu(this->trans, tmp_userid, tmp_chanid);
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
                this->send_mcs_channel_join_confirm_pdu(this->trans, this->userid, tmp_chanid);
            }

            for (size_t i = 0 ; i < this->channel_list.size() ; i++){
                    uint16_t tmp_userid;
                    uint16_t tmp_chanid;
                    this->recv_mcs_channel_join_request_pdu(this->trans, tmp_userid, tmp_chanid);
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
                    this->send_mcs_channel_join_confirm_pdu(this->trans, this->userid, tmp_chanid);
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
            Stream stream(65535);
            X224In tpdu(this->trans, stream);
            McsIn mcs_in(stream);
            if ((mcs_in.opcode >> 2) != MCS_SDRQ) {
                TODO("We should make a special case for MCS_DPUM, as this one is a demand to end connection");
                // mcs_in.opcode >> 2) == MCS_DPUM
                throw Error(ERR_MCS_APPID_NOT_MCS_SDRQ);
            }

            if (this->verbose >= 256){
                this->decrypt.dump();
            }

            SecIn sec(stream, this->decrypt);

            if (!sec.flags & SEC_LOGON_INFO) { /* 0x01 */
                throw Error(ERR_SEC_EXPECTED_LOGON_INFO);
            }

            /* this is the first test that the decrypt is working */
            this->client_info.process_logon_info(stream);
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
                sec.end();
                mcs_in.end();
                tpdu.end();
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
            Stream stream(65535);
            X224In tpdu(this->trans, stream);
            McsIn mcs_in(stream);
            if ((mcs_in.opcode >> 2) != MCS_SDRQ) {
                TODO("We should make a special case for MCS_DPUM, as this one is a demand to end connection");
                // mcs_in.opcode >> 2) == MCS_DPUM
                throw Error(ERR_MCS_APPID_NOT_MCS_SDRQ);
            }

            if (this->verbose >= 256){
                this->decrypt.dump();
            }

            SecIn sec(stream, this->decrypt);

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

            if (sec.flags & SEC_LICENCE_NEG) { /* 0x80 */
                uint8_t tag = stream.in_uint8();
                uint8_t version = stream.in_uint8();
                uint16_t length = stream.in_uint16_le();
                TODO("currently we just skip data, we should consume them instead")
                stream.p = stream.end;
                LOG(LOG_INFO, "Front::WAITING_FOR_ANSWER_TO_LICENCE sec_flags=%x %u %u %u", sec.flags, tag, version, length);

                switch (tag) {
                case LICENCE_TAG_DEMAND:
                    LOG(LOG_INFO, "Front::LICENCE_TAG_DEMAND");
                    LOG(LOG_INFO, "Front::incoming::licencing send_lic_response");
                    send_lic_response(this->trans, this->userid);
                    break;
                case LICENCE_TAG_PRESENT:
                    LOG(LOG_INFO, "Front::LICENCE_TAG_PRESENT");
                    break;
                case LICENCE_TAG_AUTHREQ:
                    LOG(LOG_INFO, "Front::LICENCE_TAG_AUTHREQ");
                    break;
                case LICENCE_TAG_ISSUE:
                    LOG(LOG_INFO, "Front::LICENCE_TAG_ISSUE");
                    break;
                case LICENCE_TAG_REISSUE:
                    LOG(LOG_INFO, "Front::LICENCE_TAG_REISSUE");
                    break;
                case LICENCE_TAG_RESULT:
                    LOG(LOG_INFO, "Front::LICENCE_TAG_RESULT");
                    break;
                case LICENCE_TAG_REQUEST:
                    LOG(LOG_INFO, "Front::LICENCE_TAG_REQUEST");
                    LOG(LOG_INFO, "Front::incoming::licencing send_lic_response");
                    send_lic_response(this->trans, this->userid);
                    break;
                case LICENCE_TAG_AUTHRESP:
                    LOG(LOG_INFO, "Front::LICENCE_TAG_AUTHRESP");
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
                ShareControlIn sci(stream);
                switch (sci.pdu_type1) {
                case PDUTYPE_DEMANDACTIVEPDU: /* 1 */
                    if (this->verbose){
                        LOG(LOG_INFO, "unexpected DEMANDACTIVE PDU while in licence negociation");
                    }
                    break;
                case PDUTYPE_CONFIRMACTIVEPDU:
                    if (this->verbose){
                        LOG(LOG_INFO, "Unexpected CONFIRMACTIVE PDU");
                    }
                    this->process_confirm_active(stream);

                    break;
                case PDUTYPE_DATAPDU: /* 7 */
                    if (this->verbose & 4){
                        LOG(LOG_INFO, "unexpected DATA PDU while in licence negociation");
                    }
                    // at this point licence negociation is still ongoing
                    // most data packets should not be received
                    // actually even input is dubious,
                    // but rdesktop actually sends input data
                    this->process_data(stream, cb);
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
                    LOG(LOG_WARNING, "unknown PDU type received while in licence negociation (%d)\n", sci.pdu_type1);
                    break;
                }
            }
            sec.end();
            mcs_in.end();
            tpdu.end();
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
            ChannelList & channel_list = this->channel_list;

            Stream stream(65535);

            X224In tpdu(this->trans, stream);

            if (tpdu.tpdu_hdr.code != X224Packet::DT_TPDU){
                TODO("we can also get a DR (Disconnect Request), this a normal case that should be managed")
                LOG(LOG_INFO, "Front::Unexpected non data PDU (got %u)", tpdu.tpdu_hdr.code);
                throw Error(ERR_X224_EXPECTED_DATA_PDU);
            }

            McsIn mcs_in(stream);

            // Disconnect Provider Ultimatum datagram
            if ((mcs_in.opcode >> 2) == MCS_DPUM) {
                throw Error(ERR_MCS_APPID_IS_MCS_DPUM);
            }

            if ((mcs_in.opcode >> 2) != MCS_SDRQ) {
                throw Error(ERR_MCS_APPID_NOT_MCS_SDRQ);
            }

            SecIn sec(stream, this->decrypt);

            if (this->verbose & 4){
                LOG(LOG_INFO, "Front::incoming::sec_flags=%x", sec.flags);
            }

            if (sec.flags & 0x0400){ /* SEC_REDIRECT_ENCRYPT */
                if (this->verbose){
                    LOG(LOG_INFO, "Front::incoming::SEC_REDIRECT_ENCRYPT");
                }
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

            if (mcs_in.chan_id != MCS_GLOBAL_CHANNEL) {
                size_t num_channel_src = channel_list.size();
                for (size_t index = 0; index < channel_list.size(); index++){
                    if (channel_list[index].chanid == mcs_in.chan_id){
                        num_channel_src = index;
                        break;
                    }
                }

                if (num_channel_src >= channel_list.size()) {
                    LOG(LOG_ERR, "Front::incoming::Unknown Channel");
                    throw Error(ERR_CHANNEL_UNKNOWN_CHANNEL);
                }

                const McsChannelItem & channel = channel_list[num_channel_src];

                int length = stream.in_uint32_le();
                int flags = stream.in_uint32_le();

                size_t chunk_size = stream.end - stream.p;

                if (this->up_and_running){
                    cb.send_to_mod_channel(channel.name, stream.p, length, chunk_size, flags);
                }
                stream.p += chunk_size;
            }
            else {
                while (stream.p < stream.end) {
                    ShareControlIn sci(stream);

                    switch (sci.pdu_type1) {
                    case PDUTYPE_DEMANDACTIVEPDU:
                        if (this->verbose){
                            LOG(LOG_INFO, "Front received DEMANDACTIVEPDU");
                        }
                        break;
                    case PDUTYPE_CONFIRMACTIVEPDU:
                        if (this->verbose){
                            LOG(LOG_INFO, "Front received CONFIRMACTIVEPDU");
                        }
                        this->process_confirm_active(stream);
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
                        this->process_data(stream, cb);
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
                        LOG(LOG_WARNING, "Front received unknown PDU type in session_data (%d)\n", sci.pdu_type1);
                        break;
                    }
                }
            }
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
        Stream stream(32768);
        X224Out tpdu(X224Packet::DT_TPDU, stream);
        McsOut sdin_out(stream, MCS_SDIN, this->userid, MCS_GLOBAL_CHANNEL);
        uint32_t sec_flags = this->client_info.crypt_level?SEC_ENCRYPT:0;
        SecOut sec_out(stream, sec_flags, this->encrypt);
        ShareControlOut rdp_control_out(stream, PDUTYPE_DATAPDU, this->userid + MCS_USERCHANNEL_BASE);
        ShareDataOut rdp_data_out(stream, PDUTYPE2_UPDATE, this->share_id, RDP::STREAM_MED);

        stream.out_uint16_le(RDP_UPDATE_SYNCHRONIZE);
        stream.out_clear_bytes(2);

        rdp_data_out.end();
        rdp_control_out.end();
        sec_out.end();
        sdin_out.end();
        tpdu.end();
        tpdu.send(this->trans);
    }



    /*****************************************************************************/
    void send_demand_active() throw (Error)
    {
        LOG(LOG_INFO, "Front::send_demand_active");

        Stream stream(32768);
        X224Out tpdu(X224Packet::DT_TPDU, stream);
        McsOut sdin_out(stream, MCS_SDIN, this->userid, MCS_GLOBAL_CHANNEL);
        uint32_t sec_flags = this->client_info.crypt_level?SEC_ENCRYPT:0;
        SecOut sec_out(stream, sec_flags, this->encrypt);
        ShareControlOut rdp_out(stream, PDUTYPE_DEMANDACTIVEPDU, this->userid + MCS_USERCHANNEL_BASE);

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

        caps_count++; front_out_share_caps(stream, this->userid + MCS_USERCHANNEL_BASE);

        caps_count++;
        GeneralCaps general;
        general.log("Sending to client");
        general.emit(stream);

        caps_count++; front_out_bitmap_caps(stream,
                                            this->client_info.bpp,
                                            this->client_info.width,
                                            this->client_info.height);

        caps_count++; front_out_font_caps(stream);
        caps_count++; sc_out_order_caps(stream);
        caps_count++; front_out_colcache_caps(stream);
        caps_count++; front_out_pointer_caps(stream);

        /* Output input capability set */
        caps_count++; front_out_input_caps(stream);

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

        rdp_out.end();
        sec_out.end();
        sdin_out.end();
        tpdu.end();
        tpdu.send(this->trans);
    }


    /* store the bitmap cache size in client_info */
    void capset_bmpcache(Stream & stream, int len)
    {
        LOG(LOG_INFO, "capset_bmpcache");
        stream.in_skip_bytes(24);
        this->client_info.cache1_entries = stream.in_uint16_le();
        this->client_info.cache1_size = stream.in_uint16_le();
        this->client_info.cache2_entries = stream.in_uint16_le();
        this->client_info.cache2_size = stream.in_uint16_le();
        this->client_info.cache3_entries = stream.in_uint16_le();
        this->client_info.cache3_size = stream.in_uint16_le();
        LOG(LOG_INFO, "cache1_entries=%d cache1_size=%d "
                      "cache2_entries=%d cache2_size=%d "
                      "cache3_entries=%d cache3_size=%d\n",
            this->client_info.cache1_entries, this->client_info.cache1_size,
            this->client_info.cache2_entries, this->client_info.cache2_size,
            this->client_info.cache3_entries, this->client_info.cache3_size);
    }

    /* store the bitmap cache size in client_info */
    void capset_bmpcache2(Stream & stream, int len)
    {
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

    /* store the number of client cursor cache in client_info */
    void capset_pointercache(Stream & stream, int len)
    {
        LOG(LOG_INFO, "capset_pointercache");
    }


    void process_confirm_active(Stream & stream)
    {
        LOG(LOG_INFO, "process_confirm_active");
        stream.in_skip_bytes(4); /* rdp_shareid */
        stream.in_skip_bytes(2); /* userid */
        int source_len = stream.in_uint16_le(); /* sizeof RDP_SOURCE */
        // int cap_len = stream.in_uint16_le();
        stream.in_skip_bytes(2); // skip cap_len
        stream.in_skip_bytes(source_len);
        int num_caps = stream.in_uint16_le();
        stream.in_skip_bytes(2); /* pad */

        for (int index = 0; index < num_caps; index++) {
            uint8_t *p = stream.p;
            int type = stream.in_uint16_le();
            int len = stream.in_uint16_le();

            switch (type) {
            case RDP_CAPSET_GENERAL:
            {
                GeneralCaps general;
                general.recv(stream);
                general.log("Receiving from client");
                this->client_info.use_compact_packets = 0!=(general.extraflags & NO_BITMAP_COMPRESSION_HDR);
            }
            break;
            case RDP_CAPSET_BITMAP: /* 2 */
                break;
            case RDP_CAPSET_ORDER: /* 3 */
                cs_in_order_caps(stream, len, this->client_info.desktop_cache);
                break;
            case RDP_CAPSET_BMPCACHE: /* 4 */
                this->capset_bmpcache(stream, len);
                break;
            case RDP_CAPSET_CONTROL: /* 5 */
                break;
            case RDP_CAPSET_ACTIVATE: /* 7 */
                break;
            case RDP_CAPSET_POINTER: /* 8 */
                {
                    stream.in_skip_bytes(2); /* color pointer */
                    int i = stream.in_uint16_le();
                    this->client_info.pointer_cache_entries = std::min(i, 32);
                }
                break;
            case RDP_CAPSET_SHARE: /* 9 */
                break;
            case RDP_CAPSET_COLCACHE: /* 10 */
                break;
            case 12: /* 12 */
                break;
            case 13: /* 13 */
                break;
            case 14: /* 14 */
                break;
            case RDP_CAPSET_BRUSHCACHE: /* 15 */
                this->client_info.brush_cache_code = stream.in_uint32_le();
                break;
            case 16: /* 16 */
                break;
            case 17: /* 17 */
                break;
            case RDP_CAPSET_BMPCACHE2: /* 19 */
                this->capset_bmpcache2(stream, len);
                break;
            case 20: /* 20 */
                break;
            case 21: /* 21 */
                break;
            case 22: /* 22 */
                break;
            case 26: /* 26 */
                break;
            default:
                break;
            }
            stream.p = p + len;
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

        Stream stream(32768);
        X224Out tpdu(X224Packet::DT_TPDU, stream);
        McsOut sdin_out(stream, MCS_SDIN, this->userid, MCS_GLOBAL_CHANNEL);
        uint32_t sec_flags = this->client_info.crypt_level?SEC_ENCRYPT:0;
        SecOut sec_out(stream, sec_flags, this->encrypt);
        ShareControlOut rdp_control_out(stream, PDUTYPE_DATAPDU, this->userid + MCS_USERCHANNEL_BASE);
        ShareDataOut rdp_data_out(stream, PDUTYPE2_SYNCHRONIZE, this->share_id, RDP::STREAM_MED);

        stream.out_uint16_le(1); /* messageType */
        stream.out_uint16_le(1002); /* control id */

        rdp_data_out.end();
        rdp_control_out.end();
        sec_out.end();
        sdin_out.end();
        tpdu.end();
        tpdu.send(this->trans);

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

        Stream stream(32768);
        X224Out tpdu(X224Packet::DT_TPDU, stream);
        McsOut sdin_out(stream, MCS_SDIN, this->userid, MCS_GLOBAL_CHANNEL);
        uint32_t sec_flags = this->client_info.crypt_level?SEC_ENCRYPT:0;
        SecOut sec_out(stream, sec_flags, this->encrypt);
        ShareControlOut rdp_control_out(stream, PDUTYPE_DATAPDU, this->userid + MCS_USERCHANNEL_BASE);
        ShareDataOut rdp_data_out(stream, PDUTYPE2_CONTROL, this->share_id, RDP::STREAM_MED);

        stream.out_uint16_le(action);
        stream.out_uint16_le(0); /* userid */
        stream.out_uint32_le(1002); /* control id */

        rdp_data_out.end();
        rdp_control_out.end();
        sec_out.end();
        sdin_out.end();
        tpdu.end();
        tpdu.send(this->trans);

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

        TODO(" we should create some RDPStream object created on init and sent before destruction")
        Stream stream(32768);
        X224Out tpdu(X224Packet::DT_TPDU, stream);
        McsOut sdin_out(stream, MCS_SDIN, this->userid, MCS_GLOBAL_CHANNEL);
        uint32_t sec_flags = this->client_info.crypt_level?SEC_ENCRYPT:0;
        SecOut sec_out(stream, sec_flags, this->encrypt);
        ShareControlOut rdp_control_out(stream, PDUTYPE_DATAPDU, this->userid + MCS_USERCHANNEL_BASE);
        ShareDataOut rdp_data_out(stream, PDUTYPE2_FONTMAP, this->share_id, RDP::STREAM_MED);

        stream.out_copy_bytes((char*)g_fontmap, 172);

        rdp_data_out.end();
        rdp_control_out.end();
        sec_out.end();
        sdin_out.end();
        tpdu.end();
        tpdu.send(this->trans);

    }

    /* PDUTYPE_DATAPDU */
    void process_data(Stream & stream, Callback & cb) throw (Error)
    {
        if (this->verbose & 4){
            LOG(LOG_INFO, "Front::process_data(...)");
        }
        ShareDataIn share_data_in(stream);
        if (this->verbose > 0x80){
            LOG(LOG_INFO, "share_data_in.pdutype2=%u"
                          " share_data_in.len=%u"
                          " share_data_in.compressedLen=%u"
                          " remains=%u",
                (unsigned)share_data_in.pdutype2,
                (unsigned)share_data_in.len,
                (unsigned)share_data_in.compressedLen,
                (unsigned)(stream.end - stream.p)
            );
        }

        switch (share_data_in.pdutype2) {
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

                BGRPalette palette;
                init_palette332(palette);
                this->color_cache(palette, 0);
                this->init_pointers();

                this->up_and_running = 1;
                if (this->verbose){
                    LOG(LOG_INFO, "--------------> UP AND RUNNING <----------------");
                }
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
                Stream stream(32768);
                X224Out tpdu(X224Packet::DT_TPDU, stream);
                McsOut sdin_out(stream, MCS_SDIN, this->userid, MCS_GLOBAL_CHANNEL);
                uint32_t sec_flags = this->client_info.crypt_level?SEC_ENCRYPT:0;
                SecOut sec_out(stream, sec_flags, this->encrypt);
                ShareControlOut rdp_control_out(stream, PDUTYPE_DATAPDU, this->userid + MCS_USERCHANNEL_BASE);
                ShareDataOut rdp_data_out(stream, PDUTYPE2_SHUTDOWN_DENIED, this->share_id, RDP::STREAM_MED);
                rdp_data_out.end();
                rdp_control_out.end();
                sec_out.end();
                sdin_out.end();
                tpdu.end();
                tpdu.send(this->trans);
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
            {
                int seq = stream.in_uint16_le();
                /* 419 client sends Seq 1, then 2 */
                /* 2600 clients sends only Seq 3 */
                /* after second font message, we are up and running */
                if (seq == 2 || seq == 3)
                {
                    this->send_fontmap();
//                    this->up_and_running = 1;
                    this->send_data_update_sync();
                }
            }
            stream.in_uint16_le(); /* entrySize -> 50 */
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
            LOG(LOG_WARNING, "unsupported PDUTYPE in process_data %d\n", share_data_in.pdutype2);
            break;
        }
        share_data_in.end();
        if (this->verbose & 4){
            LOG(LOG_INFO, "process_data done");
        }
    }

    void send_deactive() throw (Error)
    {
        LOG(LOG_INFO, "send_deactive");
        Stream stream(32768);
        X224Out tpdu(X224Packet::DT_TPDU, stream);
        McsOut sdin_out(stream, MCS_SDIN, this->userid, MCS_GLOBAL_CHANNEL);
        uint32_t sec_flags = this->client_info.crypt_level?SEC_ENCRYPT:0;
        SecOut sec_out(stream, sec_flags, this->encrypt);
        ShareControlOut(stream, PDUTYPE_DEACTIVATEALLPDU, this->userid + MCS_USERCHANNEL_BASE).end();
        sec_out.end();
        sdin_out.end();
        tpdu.end();
        tpdu.send(this->trans);
    }


    void draw(const RDPOpaqueRect & cmd, const Rect & clip)
    {
        if (!clip.isempty()
        && !clip.intersect(cmd.rect).isempty()){

            this->send_global_palette();

            RDPOpaqueRect new_cmd = cmd;
            new_cmd.color = this->convert_opaque(cmd.color);

            this->orders->draw(new_cmd, clip);

            if (this->capture){
                RDPOpaqueRect new_cmd24 = cmd;
                new_cmd24.color = this->convert24_opaque(cmd.color);
                this->capture->draw(new_cmd24, clip);
            }
        }
    }

    void draw(const RDPScrBlt & cmd, const Rect & clip)
    {
        if (!clip.isempty()
        && !clip.intersect(cmd.rect).isempty()){
            this->orders->draw(cmd, clip);

            if (this->capture){
                this->capture->draw(cmd, clip);
            }
        }
    }

    void draw(const RDPDestBlt & cmd, const Rect & clip)
    {
        if (!clip.isempty()
        && !clip.intersect(cmd.rect).isempty()){
            this->orders->draw(cmd, clip);
            if (this->capture){
                this->capture->draw(cmd, clip);
            }
        }
    }


    void draw(const RDPPatBlt & cmd, const Rect & clip)
    {
        if (!clip.isempty()
        && !clip.intersect(cmd.rect).isempty()){

            this->send_global_palette();

            RDPPatBlt new_cmd = cmd;
            new_cmd.back_color = this->convert(cmd.back_color);
            new_cmd.fore_color = this->convert(cmd.fore_color);

            TODO("Shouldn't this be done before calling draw");
            // this may change the brush add send it to to remote cache
            this->cache_brush(new_cmd.brush);

            this->orders->draw(new_cmd, clip);

            if (this->capture){
                RDPPatBlt new_cmd24 = cmd;
                new_cmd24.back_color = this->convert24(cmd.back_color);
                new_cmd24.fore_color = this->convert24(cmd.fore_color);

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
        if (src_tile == Rect(0, 0, bitmap.cx, bitmap.cy)){
            const RDPMemBlt cmd2(0, dst_tile, cmd.rop, 0, 0, 0);
            this->orders->draw(cmd2, clip, bitmap);
            if (this->capture){
                this->capture->draw(cmd2, clip, bitmap);
            }
        }
        else {
            TODO("if we immediately create tiled_bitmap at the target bpp value, we would avoid a data copy. Drawback need one more parameter to tiling bitmap constructor")
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
//        LOG(LOG_INFO, "front::draw:RDPMemBlt");
        if (bitmap.cx < cmd.srcx || bitmap.cy < cmd.srcy){
            return;
        }

        const uint8_t palette_id = 0;
        if (this->client_info.bpp == 8){
            this->palette_sent = false;
            this->send_global_palette();
            if (!this->palette_memblt_sent[palette_id]) {
                this->color_cache(bitmap.original_palette, palette_id);
                this->palette_memblt_sent[palette_id] = true;
            }
            this->palette_sent = false;
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
            && align4(dst_cx) < 256 && dst_cy < 256){
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
            new_cmd.back_color = this->convert(cmd.back_color);
            new_cmd.pen.color = this->convert(cmd.pen.color);

            this->orders->draw(new_cmd, clip);

            if (this->capture){
                RDPLineTo new_cmd24 = cmd;
                new_cmd24.back_color = this->convert24(cmd.back_color);
                new_cmd24.pen.color = this->convert24(cmd.pen.color);

                this->capture->draw(new_cmd24, clip);
            }
        }
    }

    void draw(const RDPGlyphIndex & cmd, const Rect & clip)
    {
        if (!clip.isempty() && !clip.intersect(cmd.bk).isempty()){
            this->send_global_palette();

            RDPGlyphIndex new_cmd = cmd;
            new_cmd.back_color = this->convert_opaque(cmd.back_color);
            new_cmd.fore_color = this->convert_opaque(cmd.fore_color);

            TODO("Shouldn't this be done before calling draw");
            // this may change the brush and send it to to remote cache
            this->cache_brush(new_cmd.brush);

            this->orders->draw(new_cmd, clip);

            if (this->capture){
                RDPGlyphIndex new_cmd24 = cmd;
                new_cmd24.back_color = this->convert24_opaque(cmd.back_color);
                new_cmd24.fore_color = this->convert24_opaque(cmd.fore_color);

                this->capture->draw(new_cmd24, clip);
            }
        }
    }

    void draw(const RDPGlyphCache & cmd)
    {
        this->orders->draw(cmd);
    }

    void flush(){}

    void color_cache(const BGRPalette & palette, uint8_t cacheIndex)
    {
        RDPColCache cmd(cacheIndex, palette);
        this->orders->draw(cmd);
    }

    void cache_brush(RDPBrush & brush)
    {
        if ((brush.style == 3)
        && (this->client_info.brush_cache_code == 1)) {
            uint8_t pattern[8];
            pattern[0] = brush.hatch;
            memcpy(pattern+1, brush.extra, 7);
            int cache_idx = 0;
            if (BRUSH_TO_SEND == this->cache.add_brush(pattern, cache_idx)){
                RDPBrushCache cmd(cache_idx, 1, 8, 8, 0x81,
                    sizeof(this->cache.brush_items[cache_idx].pattern),
                    this->cache.brush_items[cache_idx].pattern);
                this->orders->draw(cmd);
            }
            brush.hatch = cache_idx;
            brush.style = 0x81;
        }
    }

    const BGRColor convert24_opaque(const BGRColor color) const
    {
        if (this->mod_bpp == 16 || this->mod_bpp == 15){
            const BGRColor color24 = color_decode_opaquerect(
                        color, this->mod_bpp, this->mod_palette);
            return  color_encode(color24, 24);
        }
        else if (this->mod_bpp == 8) {
            const BGRColor color24 = color_decode(color, this->mod_bpp, this->mod_palette);
            return RGBtoBGR(color_encode(color24, 24));
        }
        else {
            const BGRColor color24 = color_decode(color, this->mod_bpp, this->mod_palette);
            return color_encode(color24, 24);
        }
    }

    const BGRColor convert(const BGRColor color) const
    {
        if (this->client_info.bpp == 8 && this->mod_bpp == 8){
//            return ((color >> 5) & 7) |((color << 1) & 0x31)|((color<<6)&0xc0);
//            this->mod_palette[color]
            return color;
        }
        else{
            const BGRColor color24 = color_decode(color, this->mod_bpp, this->mod_palette);
            return color_encode(color24, this->client_info.bpp);
        }
    }


    const BGRColor convert24(const BGRColor color) const
    {
        const BGRColor color24 = color_decode(color, this->mod_bpp, this->mod_palette);
        return color_encode(color24, 24);
    }

    const BGRColor convert_opaque(const BGRColor color) const
    {
        if (this->client_info.bpp == 8 && this->mod_bpp == 8){
//            LOG(LOG_INFO, "convert_opaque: front=%u back=%u setted=%u color=%u palette=%.06x", this->client_info.bpp, this->front.mod_bpp, this->mod_palette_setted, color, this->mod_palette[color]);
//            return ((color >> 5) & 7) |((color << 1) & 0x31)|((color<<6)&0xc0);
//            this->mod_palette[color]
            return color;
        }
        else
        if (this->mod_bpp == 16 || this->mod_bpp == 15 || this->mod_bpp == 8){
            const BGRColor color24 = color_decode_opaquerect(
                        color, this->mod_bpp, this->mod_palette);
            return  color_encode(color24, this->client_info.bpp);
        }
        else {
            const BGRColor color24 = color_decode(color, this->mod_bpp, this->mod_palette);
            return color_encode(color24, this->client_info.bpp);
        }
    }

    TODO("RGBtoBGR conversion should be done by caller when necessary. Also we should separate memblt palette and global palette")
    void set_mod_palette(const BGRPalette & palette)
    {
        this->mod_palette_setted = true;
        for (unsigned i = 0; i < 256 ; i++){
            this->mod_palette[i] = palette[i];
            this->memblt_mod_palette[i] = RGBtoBGR(palette[i]);
        }
    }

    void set_mod_bpp_to_front_bpp()
    {
        this->mod_bpp = this->client_info.bpp;
    }

};

#endif
