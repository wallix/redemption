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

#if !defined(__FRONT_HPP__)
#define __FRONT_HPP__

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
#include "file_loc.hpp"
#include "altoco.hpp"
#include "rect.hpp"
#include "region.hpp"
#include "capture.hpp"
#include "font.hpp"
#include "bitmap.hpp"
#include "bitmap_cache.hpp"
#include "cache.hpp"
#include "client_info.hpp"
#include "config.hpp"
#include "error.hpp"
#include "callback.hpp"
#include "colors.hpp"
#include "altoco.hpp"
#include "transport.hpp"

#include "RDP/x224.hpp"
#include "RDP/mcs.hpp"
#include "RDP/rdp.hpp"
#include "RDP/sec.hpp"
#include "RDP/lic.hpp"
#include "RDP/orders/RDPOrdersCommon.hpp"
#include "RDP/orders/RDPOrdersPrimaryHeader.hpp"
#include "RDP/orders/RDPOrdersPrimaryOpaqueRect.hpp"
#include "RDP/orders/RDPOrdersPrimaryScrBlt.hpp"
#include "RDP/orders/RDPOrdersPrimaryDestBlt.hpp"
#include "RDP/orders/RDPOrdersPrimaryMemBlt.hpp"
#include "RDP/orders/RDPOrdersPrimaryPatBlt.hpp"
#include "RDP/orders/RDPOrdersPrimaryLineTo.hpp"
#include "RDP/orders/RDPOrdersPrimaryGlyphIndex.hpp"
#include "RDP/orders/RDPOrdersSecondaryHeader.hpp"
#include "RDP/orders/RDPOrdersSecondaryColorCache.hpp"
#include "RDP/orders/RDPOrdersSecondaryBmpCache.hpp"
#include "RDP/orders/RDPOrdersSecondaryBrushCache.hpp"
#include "RDP/orders/RDPOrdersSecondaryGlyphCache.hpp"



// MS-RDPECGI 2.2.2.2 Fast-Path Orders Update (TS_FP_UPDATE_ORDERS)
// ================================================================
// The TS_FP_UPDATE_ORDERS structure contains primary, secondary, and alternate
// secondary drawing orders aligned on byte boundaries. This structure conforms
// to the layout of a Fast-Path Update (see [MS-RDPBCGR] section 2.2.9.1.2.1)
// and is encapsulated within a Fast-Path Update PDU (see [MS-RDPBCGR] section
// 2.2.9.1.2.1.1).

// updateHeader (1 byte): An 8-bit, unsigned integer. The format of this field
//   is the same as the updateHeader byte field described in the Fast-Path
//   Update structure (see [MS-RDPBCGR] section 2.2.9.1.2.1). The updateCode
//   bitfield (4 bits in size) MUST be set to FASTPATH_UPDATETYPE_ORDERS (0x0).

// compressionFlags (1 byte): An 8-bit, unsigned integer. The format of this
//   optional field (as well as the possible values) is the same as the
//   compressionFlags field described in the Fast-Path Update structure
//   specified in [MS-RDPBCGR] section 2.2.9.1.2.1.

// size (2 bytes): A 16-bit, unsigned integer. The format of this field (as well
//   as the possible values) is the same as the size field described in the
//   Fast-Path Update structure specified in [MS-RDPBCGR] section 2.2.9.1.2.1.

// numberOrders (2 bytes): A 16-bit, unsigned integer. The number of Drawing
//   Order (section 2.2.2.1.1) structures contained in the orderData field.

// orderData (variable): A variable-sized array of Drawing Order (section
//   2.2.2.1.1) structures packed on byte boundaries. Each structure contains a
//   primary, secondary, or alternate secondary drawing order. The controlFlags
//   field of the Drawing Order identifies the type of drawing order.


// MS-RDPECGI 2.2.2.1 Orders Update (TS_UPDATE_ORDERS_PDU_DATA)
// ============================================================
// The TS_UPDATE_ORDERS_PDU_DATA structure contains primary, secondary, and
// alternate secondary drawing orders aligned on byte boundaries. This structure
// conforms to the layout of a Slow Path Graphics Update (see [MS-RDPBCGR]
// section 2.2.9.1.1.3.1) and is encapsulated within a Graphics Update PDU (see
// [MS-RDPBCGR] section 2.2.9.1.1.3.1.1).

// shareDataHeader (18 bytes): Share Data Header (see [MS-RDPBCGR], section
//   2.2.8.1.1.1.2) containing information about the packet. The type subfield
//   of the pduType field of the Share Control Header (section 2.2.8.1.1.1.1)
//   MUST be set to PDUTYPE_DATAPDU (7). The pduType2 field of the Share Data
//   Header MUST be set to PDUTYPE2_UPDATE (2).

// updateType (2 bytes): A 16-bit, unsigned integer. The field contains the
//   graphics update type. This field MUST be set to UPDATETYPE_ORDERS (0x0000).

// pad2OctetsA (2 bytes): A 16-bit, unsigned integer used as a padding field.
//   Values in this field are arbitrary and MUST be ignored.

// numberOrders (2 bytes): A 16-bit, unsigned integer. The number of Drawing
//   Order (section 2.2.2.1.1) structures contained in the orderData field.

// pad2OctetsB (2 bytes): A 16-bit, unsigned integer used as a padding field.
//   Values in this field are arbitrary and MUST be ignored.

// orderData (variable): A variable-sized array of Drawing Order (section
//   2.2.2.1.1) structures packed on byte boundaries. Each structure contains a
//   primary, secondary, or alternate secondary drawing order. The controlFlags
//   field of the Drawing Order identifies the type of drawing order.

struct GraphicsUpdatePDU
{
    Stream stream;
    // Internal state of orders
    RDPOrderCommon common;
    RDPDestBlt destblt;
    RDPPatBlt patblt;
    RDPScrBlt scrblt;
    RDPOpaqueRect opaquerect;
    RDPMemBlt memblt;
    RDPLineTo lineto;
    RDPGlyphIndex glyphindex;
    // state variables for gathering batch of orders
    size_t order_count;
    uint32_t offset_order_count;
    X224Out * tpdu;
    McsOut * mcs_sdin;
    SecOut * sec_out;
    ShareControlOut * out_control;
    ShareDataOut * out_data;
    Transport * trans;
    uint16_t & userid;
    int & shareid;
    int & crypt_level;
    CryptContext & encrypt;

    GraphicsUpdatePDU(Transport * trans,
                      uint16_t & userid,
                      int & shareid,
                      int & crypt_level,
                      CryptContext & encrypt)
        :    stream(4096),
        // Internal state of orders
        common(RDP::PATBLT, Rect(0, 0, 1, 1)),
        destblt(Rect(), 0),
        patblt(Rect(), 0, 0, 0, RDPBrush()),
        scrblt(Rect(), 0, 0, 0),
        opaquerect(Rect(), 0),
        memblt(0, Rect(), 0, 0, 0, 0),
        lineto(0, 0, 0, 0, 0, 0, 0, RDPPen(0, 0, 0)),
        glyphindex(0, 0, 0, 0, 0, 0, Rect(0, 0, 1, 1), Rect(0, 0, 1, 1), RDPBrush(), 0, 0, 0, (uint8_t*)""),
        // state variables for a batch of orders
        order_count(0),
        offset_order_count(0),
        tpdu(NULL),
        mcs_sdin(NULL),
        sec_out(0),
        out_control(NULL),
        out_data(NULL),
        trans(trans),
        userid(userid),
        shareid(shareid),
        crypt_level(crypt_level),
        encrypt(encrypt)
    {
        this->init();
    }

    ~GraphicsUpdatePDU(){
        if (this->tpdu){ delete this->tpdu; }
        if (this->mcs_sdin){ delete this->mcs_sdin; }
        if (this->sec_out){ delete this->sec_out; }
        if (this->out_control){ delete this->out_control; }
        if (this->out_data){ delete this->out_data; }
    }

    void init(){
        if (this->tpdu){ delete this->tpdu; }
        if (this->mcs_sdin){ delete this->mcs_sdin; }
        if (this->sec_out){ delete this->sec_out; }
        if (this->out_control){ delete this->out_control; }
        if (this->out_data){ delete this->out_data; }

        this->stream.init(4096);
        this->tpdu = new X224Out(X224Packet::DT_TPDU, this->stream);
        this->mcs_sdin = new McsOut(stream, MCS_SDIN, this->userid, MCS_GLOBAL_CHANNEL);
        this->sec_out = new SecOut(stream, this->crypt_level, SEC_ENCRYPT, this->encrypt);
        this->out_control = new ShareControlOut(this->stream, PDUTYPE_DATAPDU, this->userid + MCS_USERCHANNEL_BASE);
        this->out_data = new ShareDataOut(this->stream, PDUTYPE2_UPDATE, this->shareid);

        this->stream.out_uint16_le(RDP_UPDATE_ORDERS);
        this->stream.out_clear_bytes(2); /* pad */
        this->offset_order_count = this->stream.get_offset(0);
        this->stream.out_clear_bytes(2); /* number of orders, set later */
        this->stream.out_clear_bytes(2); /* pad */
    }

    void flush()
    {
        if (this->order_count > 0){
//            LOG(LOG_ERR, "GraphicsUpdatePDU::flush: order_count=%d", this->order_count);
            this->stream.set_out_uint16_le(this->order_count, this->offset_order_count);
            this->order_count = 0;

            this->out_data->end();
            this->out_control->end();
            this->sec_out->end();
            this->mcs_sdin->end();
            this->tpdu->end();
            this->tpdu->send(this->trans);
            this->init();
        }
    }

    /*****************************************************************************/
    // check if the next order will fit in available packet size
    // if not send previous orders we got and init a new packet
    void reserve_order(size_t asked_size)
    {
//        LOG(LOG_INFO, "reserve_order[%u](%u) remains=%u", this->order_count, asked_size, std::min(this->stream.capacity, (size_t)4096) - this->stream.get_offset(0));
        size_t max_packet_size = std::min(this->stream.capacity, (size_t)4096);
        size_t used_size = this->stream.get_offset(0);
        const size_t max_order_batch = 4096;
        if ((this->order_count >= max_order_batch)
        || (used_size + asked_size + 100) > max_packet_size) {
            this->flush();
        }
        this->order_count++;
    }

    void send(const RDPOpaqueRect & cmd, const Rect & clip)
    {
        this->reserve_order(23);
        RDPOrderCommon newcommon(RDP::RECT, clip);
        cmd.emit(this->stream, newcommon, this->common, this->opaquerect);
        this->common = newcommon;
        this->opaquerect = cmd;
//        cmd.log(LOG_INFO, common.clip);
    }

    void send(const RDPScrBlt & cmd, const Rect &clip)
    {
        this->reserve_order(25);
        RDPOrderCommon newcommon(RDP::SCREENBLT, clip);
        cmd.emit(this->stream, newcommon, this->common, this->scrblt);
        this->common = newcommon;
        this->scrblt = cmd;
//        cmd.log(LOG_INFO, common.clip);
    }

    void send(const RDPDestBlt & cmd, const Rect &clip)
    {
        this->reserve_order(21);
        RDPOrderCommon newcommon(RDP::DESTBLT, clip);
        cmd.emit(this->stream, newcommon, this->common, this->destblt);
        this->common = newcommon;
        this->destblt = cmd;
//        cmd.log(LOG_INFO, common.clip);
    }

    void send(const RDPPatBlt & cmd, const Rect &clip)
    {
        this->reserve_order(29);
        using namespace RDP;
        RDPOrderCommon newcommon(RDP::PATBLT, clip);
        cmd.emit(this->stream, newcommon, this->common, this->patblt);
        this->common = newcommon;
        this->patblt = cmd;
//        cmd.log(LOG_INFO, common.clip);
    }


    void send(const RDPMemBlt & cmd, const Rect & clip)
    {
        this->reserve_order(30);
        RDPOrderCommon newcommon(RDP::MEMBLT, clip);
        cmd.emit(this->stream, newcommon, this->common, this->memblt);
        this->common = newcommon;
        this->memblt = cmd;
//        cmd.log(LOG_INFO, common.clip);
    }

    void send(const RDPLineTo& cmd, const Rect & clip)
    {
        this->reserve_order(32);
        RDPOrderCommon newcommon(RDP::LINE, clip);
        cmd.emit(this->stream, newcommon, this->common, this->lineto);
        this->common = newcommon;
        this->lineto = cmd;
//        cmd.log(LOG_INFO, common.clip);
    }

    void send(const RDPGlyphIndex & cmd, const Rect & clip)
    {
            this->reserve_order(297);
            RDPOrderCommon newcommon(RDP::GLYPHINDEX, clip);
            cmd.emit(this->stream, newcommon, this->common, this->glyphindex);
            this->common = newcommon;
            this->glyphindex = cmd;
    }

    void send(const RDPBrushCache & cmd)
    {
        this->reserve_order(cmd.size + 12);
        cmd.emit(this->stream);
//        cmd.log(LOG_INFO);
    }

    void send(const RDPColCache & cmd)
    {
        this->reserve_order(2000);
        cmd.emit(this->stream);
//        cmd.log(LOG_INFO);
    }

    void send(const RDPBmpCache & cmd)
    {
        this->reserve_order(cmd.bmp->bmp_size(cmd.bpp) + 16);
        cmd.emit(this->stream);
//        cmd.log(LOG_INFO);
    }

    void send(const RDPGlyphCache & cmd)
    {
        #warning compute actual size, instead of a majoration as below
        this->reserve_order(1000);
        cmd.emit(this->stream);
//        cmd.log(LOG_INFO);
    }

};

#warning front is becoming an empty shell and should disappear soon
class Front {
public:
    int (& keys)[256];
    int & key_flags;
    Keymap * &keymap;

    Cache cache;
    struct BitmapCache *bmp_cache;
    struct Font font;
    int mouse_x;
    int mouse_y;
    bool nomouse;
    bool notimestamp;
    int timezone;
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
    uint8_t client_random[64];
    uint8_t client_crypt_random[512];
    CryptContext encrypt, decrypt;

    int order_level;
    GraphicsUpdatePDU * orders;

public:

    Front(SocketTransport * trans, Inifile * ini, int (& keys)[256], int & key_flags, Keymap * &keymap) :
        keys(keys),
        key_flags(key_flags),
        keymap(keymap),
        cache(),
        bmp_cache(0),
        font(SHARE_PATH "/" DEFAULT_FONT_NAME),
        mouse_x(0),
        mouse_y(0),
        nomouse(ini->globals.nomouse),
        notimestamp(ini->globals.notimestamp),
        timezone(0),
        up_and_running(0),
        share_id(65538),
        client_info(ini),
        packet_number(1),
        trans(trans),
        userid(0),
        order_level(0)
    {
        // from server_sec
        // CGR: see if init has influence for the 3 following fields
        memset(this->server_random, 0, 32);
        memset(this->client_random, 0, 64);

        // from rdp_sec
        memset(this->client_crypt_random, 0, 512);

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

        this->orders = new GraphicsUpdatePDU(trans,
                            this->userid,
                            this->share_id,
                            this->client_info.crypt_level,
                            this->encrypt);
    }

    ~Front(){
        if (this->bmp_cache){
            delete this->bmp_cache;
        }
    }

    enum {
        CHANNEL_CHUNK_LENGTH = 8192,
        CHANNEL_FLAG_FIRST = 0x01,
        CHANNEL_FLAG_LAST = 0x02,
        CHANNEL_FLAG_SHOW_PROTOCOL = 0x10,
    };

    const ClientInfo & get_client_info() const {
        return this->client_info;
    }

    void set_client_info(uint16_t width, uint16_t height, uint8_t bpp)
    {
        this->client_info.width = width;
        this->client_info.height = height;
        this->client_info.bpp = bpp;
    }

    void reset(){
//        LOG(LOG_INFO, "reset()");
        #warning is it necessary (or even useful) to send remaining drawing orders before resetting ?
        this->orders->flush();

        /* shut down the rdp client */
        this->send_deactive();

        /* this should do the resizing */
        this->send_demand_active();

        delete this->orders;
        this->orders = new GraphicsUpdatePDU(trans, this->userid, this->share_id, this->client_info.crypt_level, this->encrypt);

        if (this->bmp_cache){
            delete this->bmp_cache;
        }
        this->bmp_cache = new BitmapCache(&(this->client_info));
        this->cache.reset(this->client_info);
    }

    void begin_update()
    {
//        LOG(LOG_INFO, "begin_update()");
        this->order_level++;
    }

    void end_update()
    {
//        LOG(LOG_INFO, "end_update()");
        this->order_level--;
        if (this->order_level == 0){
            this->orders->flush();
        }
    }

    void disconnect() throw (Error)
    {
        Stream stream(8192);
        X224Out tpdu(X224Packet::DT_TPDU, stream);

        stream.out_uint8((MCS_DPUM << 2) | 1);
        stream.out_uint8(0x80);

        tpdu.end();
        tpdu.send(this->trans);
    }

    void set_console_session(bool b)
    {
//        LOG(LOG_INFO, "set_console_session");
        this->client_info.console_session = b;
//        LOG(LOG_INFO, "set_console_session done");
    }

    const ChannelList & get_channel_list(void) const
    {
        return this->channel_list;
    }

    void send_to_channel(const McsChannelItem & channel, uint8_t* data, size_t length, int flags)
    {
        Stream stream(65536);
        X224Out tpdu(X224Packet::DT_TPDU, stream);
        McsOut sdin_out(stream, MCS_SDIN, this->userid, channel.chanid);
        SecOut sec_out(stream, this->client_info.crypt_level, SEC_ENCRYPT, this->encrypt);

        stream.out_uint32_le(length);
        if (channel.flags & CHANNEL_OPTION_SHOW_PROTOCOL) {
            flags |= CHANNEL_FLAG_SHOW_PROTOCOL;
        }
        stream.out_uint32_le(flags);
        stream.out_copy_bytes(data, length);

        sec_out.end();
        sdin_out.end();
        tpdu.end();
        tpdu.send(this->trans);
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

    void send_global_palette(const BGRPalette & palette) throw (Error)
    {
        Stream stream(8192);
        X224Out tpdu(X224Packet::DT_TPDU, stream);
        McsOut sdin_out(stream, MCS_SDIN, this->userid, MCS_GLOBAL_CHANNEL);
        SecOut sec_out(stream, this->client_info.crypt_level, SEC_ENCRYPT, this->encrypt);
        ShareControlOut rdp_control_out(stream, PDUTYPE_DATAPDU, this->userid + MCS_USERCHANNEL_BASE);
        ShareDataOut rdp_data_out(stream, PDUTYPE2_UPDATE, this->share_id);

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

    void send_pointer(int cache_idx, uint8_t* data, uint8_t* mask, int x, int y) throw (Error)
    {
        Stream stream(8192);
        X224Out tpdu(X224Packet::DT_TPDU, stream);
        McsOut sdin_out(stream, MCS_SDIN, this->userid, MCS_GLOBAL_CHANNEL);
        SecOut sec_out(stream, this->client_info.crypt_level, SEC_ENCRYPT, this->encrypt);
        ShareControlOut rdp_control_out(stream, PDUTYPE_DATAPDU, this->userid + MCS_USERCHANNEL_BASE);
        ShareDataOut rdp_data_out(stream, PDUTYPE2_POINTER, this->share_id);

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

    void set_pointer(int cache_idx) throw (Error)
    {
        Stream stream(8192);
        X224Out tpdu(X224Packet::DT_TPDU, stream);
        McsOut sdin_out(stream, MCS_SDIN, this->userid, MCS_GLOBAL_CHANNEL);
        SecOut sec_out(stream, this->client_info.crypt_level, SEC_ENCRYPT, this->encrypt);
        ShareControlOut rdp_control_out(stream, PDUTYPE_DATAPDU, this->userid + MCS_USERCHANNEL_BASE);
        ShareDataOut rdp_data_out(stream, PDUTYPE2_POINTER, this->share_id);

        stream.out_uint16_le(RDP_POINTER_CACHED);
        stream.out_uint16_le(0); /* pad */
        stream.out_uint16_le(cache_idx);

        rdp_data_out.end();
        rdp_control_out.end();
        sec_out.end();
        sdin_out.end();
        tpdu.end();
        tpdu.send(this->trans);

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


    void incoming() throw (Error)
    {
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

        recv_x224_connection_request_pdu(this->trans);
        send_x224_connection_confirm_pdu(this->trans);

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

        LOG(LOG_INFO, "Basic Settings Exchange");
        LOG(LOG_INFO, "front:basic_settings:channel_list : %u", this->channel_list.size());

        recv_mcs_connect_initial_pdu_with_gcc_conference_create_request(
            this->trans,
            &this->client_info,
            this->channel_list);

        send_mcs_connect_response_pdu_with_gcc_conference_create_response(
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

        LOG(LOG_INFO, "Channel Connection");
        recv_mcs_erect_domain_and_attach_user_request_pdu(this->trans, this->userid);

        send_mcs_attach_user_confirm_pdu(this->trans, this->userid);

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
            send_mcs_channel_join_confirm_pdu(this->trans, this->userid, tmp_chanid);
        }


        {
            uint16_t tmp_userid;
            uint16_t tmp_chanid;
            recv_mcs_channel_join_request_pdu(this->trans, tmp_userid, tmp_chanid);
            if (tmp_userid != this->userid){
                LOG(LOG_INFO, "MCS error bad userid, expecting %u got %u", this->userid, tmp_userid);
                throw Error(ERR_MCS_BAD_USERID);
            }
            if (tmp_chanid != MCS_GLOBAL_CHANNEL){
                LOG(LOG_INFO, "MCS error bad chanid expecting %u got %u", MCS_GLOBAL_CHANNEL, tmp_chanid);
                throw Error(ERR_MCS_BAD_CHANID);
            }
            send_mcs_channel_join_confirm_pdu(this->trans, this->userid, tmp_chanid);
        }


        for (size_t i = 0 ; i < this->channel_list.size() ; i++){
                uint16_t tmp_userid;
                uint16_t tmp_chanid;
                recv_mcs_channel_join_request_pdu(this->trans, tmp_userid, tmp_chanid);
                if (tmp_userid != this->userid){
                    LOG(LOG_INFO, "MCS error bad userid, expecting %u got %u", this->userid, tmp_userid);
                    throw Error(ERR_MCS_BAD_USERID);
                }
                if (tmp_chanid != this->channel_list[i].chanid){
                    LOG(LOG_INFO, "MCS error bad chanid expecting %u got %u", this->channel_list[i].chanid, tmp_chanid);
                    throw Error(ERR_MCS_BAD_CHANID);
                }
                send_mcs_channel_join_confirm_pdu(this->trans, this->userid, tmp_chanid);
                this->channel_list.set_chanid(i, tmp_chanid);
        }
        LOG(LOG_INFO, "RDP Security Commencement");

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

        recv_security_exchange_PDU(this->trans,
            this->decrypt, this->client_crypt_random);

        ssl_mod_exp(this->client_random, 64,
                    this->client_crypt_random, 64,
                    this->pub_mod, 64,
                    this->pri_exp, 64);

        // beware order of parameters for key generation (decrypt/encrypt) is inversed between server and client
        #warning looks like decrypt sign key is never used, if it's true remove it from CryptContext
        #warning this methode should probably move to ssl_calls
        rdp_sec_generate_keys(
            this->decrypt,
            this->encrypt,
            this->encrypt.sign_key,
            this->client_random,
            this->server_random,
            this->encrypt.rc4_key_size);


        // Secure Settings Exchange
        // ------------------------

        // Secure Settings Exchange: Secure client data (such as the username,
        // password and auto-reconnect cookie) is sent to the server using the Client
        // Info PDU.

        // Client                                                     Server
        //    |------ Client Info PDU      ---------------------------> |

        {
            Stream stream(8192);
            X224In tpdu(this->trans, stream);
            McsIn mcs_in(stream);

            if ((mcs_in.opcode >> 2) != MCS_SDRQ) {
                throw Error(ERR_MCS_APPID_NOT_MCS_SDRQ);
            }

            SecIn sec(stream, this->decrypt);

            if (!sec.flags & SEC_LOGON_INFO) { /* 0x01 */
                throw Error(ERR_SEC_EXPECTED_LOGON_INFO);
            }

            this->client_info.process_logon_info(stream);
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

        if (this->client_info.is_mce) {
            send_media_lic_response(this->trans, this->userid);
        }
        else {
            send_lic_initial(this->trans, this->userid);

            Stream stream(65535);
            X224In tpdu(this->trans, stream);
            McsIn mcs_in(stream);

            // Disconnect Provider Ultimatum datagram
            if ((mcs_in.opcode >> 2) == MCS_DPUM) {
                throw Error(ERR_MCS_APPID_IS_MCS_DPUM);
            }

            if ((mcs_in.opcode >> 2) != MCS_SDRQ) {
                throw Error(ERR_MCS_APPID_NOT_MCS_SDRQ);
            }

            SecIn sec(stream, this->decrypt);

            if (!sec.flags & SEC_LICENCE_NEG) { /* 0x80 */
                throw Error(ERR_SEC_EXPECTED_LICENCE_NEG);
            }
            send_lic_response(this->trans, this->userid);
        }

        // Capabilities Exchange
        // ---------------------

        // Capabilities Negotiation: The server sends the set of capabilities it
        // supports to the client in a Demand Active PDU. The client responds with its
        // capabilities by sending a Confirm Active PDU.

        // Client                                                     Server
        //    | <------- Demand Active PDU ---------------------------- |
        //    |--------- Confirm Active PDU --------------------------> |

        this->send_demand_active();

    }


    void activate_and_process_data(Callback & cb)
    {
        ChannelList & channel_list = this->channel_list;
        Stream stream(65535);

        X224In tpdu(this->trans, stream);
        McsIn mcs_in(stream);

        // Disconnect Provider Ultimatum datagram
        if ((mcs_in.opcode >> 2) == MCS_DPUM) {
            throw Error(ERR_MCS_APPID_IS_MCS_DPUM);
        }

        if ((mcs_in.opcode >> 2) != MCS_SDRQ) {
            throw Error(ERR_MCS_APPID_NOT_MCS_SDRQ);
        }

        SecIn sec(stream, this->decrypt);

        if (mcs_in.chan_id != MCS_GLOBAL_CHANNEL) {

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

            size_t index = channel_list.size();
            for (size_t i = 0; i < channel_list.size(); i++){
                if (channel_list[i].chanid == mcs_in.chan_id){
                    index = i;
                }
            }

            if (index >= channel_list.size()) {
                throw Error(ERR_CHANNEL_UNKNOWN_CHANNEL);
            }

            const McsChannelItem & channel = channel_list[index];

            int length = stream.in_uint32_le();
            int flags = stream.in_uint32_le();

            size_t chunk_size = stream.end - stream.p;

            cb.send_to_mod_channel(channel, stream.p, length, chunk_size, flags);
            stream.p += chunk_size;
        }
        else {
            while (stream.p < stream.end) {
                #warning here should be a ShareControlHeader/ShareDataHeader, check
                int length = stream.in_uint16_le();
                uint8_t * next_packet = stream.p + length;
                if (length == 0x8000) {
                    next_packet = next_packet - 0x8000 + 8;
                }
                else {
                    int pdu_code = stream.in_uint16_le();
                    stream.skip_uint8(2); /* mcs user id */
                    switch (pdu_code & 0xf) {

                    case 0:
//                        LOG(LOG_INFO, "PDUTYPE_ZERO");
                        break;
                    case PDUTYPE_DEMANDACTIVEPDU: /* 1 */
//                        LOG(LOG_INFO, "PDUTYPE_DEMANDACTIVEPDU");
                        break;
                    case PDUTYPE_CONFIRMACTIVEPDU:
//                        LOG(LOG_INFO, "PDUTYPE_CONFIRMACTIVEPDU");
                        this->process_confirm_active(stream);
                        break;
                    case PDUTYPE_DATAPDU: /* 7 */
//                        LOG(LOG_INFO, "PDUTYPE_DATAPDU");
                        // this is rdp_process_data that will set up_and_running to 1
                        // when fonts have been received
                        // we will not exit this loop until we are in this state.
                        #warning see what happen if we never receive up_and_running due to some error in client code ?
                        this->process_data(stream, cb);
//                        LOG(LOG_INFO, "PROCESS_DATA_DONE");
                        break;
                    case PDUTYPE_DEACTIVATEALLPDU:
                        LOG(LOG_WARNING, "unsupported PDU DEACTIVATEALLPDU in session_data (%d)\n", pdu_code & 0xf);
                        break;
                    case PDUTYPE_SERVER_REDIR_PKT:
                        LOG(LOG_WARNING, "unsupported PDU SERVER_REDIR_PKT in session_data (%d)\n", pdu_code & 0xf);
                        break;
                    default:
                        LOG(LOG_WARNING, "unknown PDU type in session_data (%d)\n", pdu_code & 0xf);
                        break;
                    }
                }
                next_packet = stream.p + length;
            }
        }
    }


    /*****************************************************************************/
    void send_data_update_sync() throw (Error)
    {
        LOG(LOG_INFO, "send_data_update_sync");
        Stream stream(8192);
        X224Out tpdu(X224Packet::DT_TPDU, stream);
        McsOut sdin_out(stream, MCS_SDIN, this->userid, MCS_GLOBAL_CHANNEL);
        SecOut sec_out(stream, this->client_info.crypt_level, SEC_ENCRYPT, this->encrypt);
        ShareControlOut rdp_control_out(stream, PDUTYPE_DATAPDU, this->userid + MCS_USERCHANNEL_BASE);
        ShareDataOut rdp_data_out(stream, PDUTYPE2_UPDATE, this->share_id);

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
        Stream stream(8192);
        X224Out tpdu(X224Packet::DT_TPDU, stream);
        McsOut sdin_out(stream, MCS_SDIN, this->userid, MCS_GLOBAL_CHANNEL);
        SecOut sec_out(stream, this->client_info.crypt_level, SEC_ENCRYPT, this->encrypt);
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

        /* Output share capability set */
        caps_count++;
        stream.out_uint16_le(RDP_CAPSET_SHARE);
        stream.out_uint16_le(RDP_CAPLEN_SHARE);
        stream.out_uint16_le(this->userid + MCS_USERCHANNEL_BASE);
        stream.out_uint16_be(0xb5e2); /* 0x73e1 */

        /* Output general capability set */
        caps_count++;
        stream.out_uint16_le(RDP_CAPSET_GENERAL); /* 1 */
        stream.out_uint16_le(RDP_CAPLEN_GENERAL); /* 24(0x18) */
        stream.out_uint16_le(1); /* OS major type */
        stream.out_uint16_le(3); /* OS minor type */
        stream.out_uint16_le(0x200); /* Protocol version */
        stream.out_uint16_le(0); /* pad */
        stream.out_uint16_le(0); /* Compression types */
        stream.out_uint16_le(0); /* pad use 0x40d for rdp packets, 0 for not */
        stream.out_uint16_le(0); /* Update capability */
        stream.out_uint16_le(0); /* Remote unshare capability */
        stream.out_uint16_le(0); /* Compression level */
        stream.out_uint16_le(0); /* Pad */

        /* Output bitmap capability set */
        caps_count++;
        stream.out_uint16_le(RDP_CAPSET_BITMAP); /* 2 */
        stream.out_uint16_le(RDP_CAPLEN_BITMAP); /* 28(0x1c) */
        stream.out_uint16_le(this->client_info.bpp); /* Preferred BPP */
        stream.out_uint16_le(1); /* Receive 1 BPP */
        stream.out_uint16_le(1); /* Receive 4 BPP */
        stream.out_uint16_le(1); /* Receive 8 BPP */
        stream.out_uint16_le(this->client_info.width); /* width */
        stream.out_uint16_le(this->client_info.height); /* height */
        stream.out_uint16_le(0); /* Pad */
        stream.out_uint16_le(1); /* Allow resize */
        stream.out_uint16_le(1); /* bitmap compression */
        stream.out_uint16_le(0); /* unknown */
        stream.out_uint16_le(0); /* unknown */
        stream.out_uint16_le(0); /* pad */

        /* Output font capability set */
        caps_count++;
        stream.out_uint16_le(RDP_CAPSET_FONT); /* 14 */
        stream.out_uint16_le(RDP_CAPLEN_FONT); /* 4 */

        /* Output order capability set */
        caps_count++;
        stream.out_uint16_le(RDP_CAPSET_ORDER); /* 3 */
        stream.out_uint16_le(RDP_CAPLEN_ORDER); /* 88(0x58) */
        stream.out_clear_bytes(16);
        stream.out_uint32_be(0x40420f00);
        stream.out_uint16_le(1); /* Cache X granularity */
        stream.out_uint16_le(20); /* Cache Y granularity */
        stream.out_uint16_le(0); /* Pad */
        stream.out_uint16_le(1); /* Max order level */
        stream.out_uint16_le(0x2f); /* Number of fonts */
        stream.out_uint16_le(0x22); /* Capability flags */
        /* caps */
        stream.out_uint8(1); /* dest blt */
        stream.out_uint8(1); /* pat blt */
        stream.out_uint8(1); /* screen blt */
        stream.out_uint8(1); /* mem blt */
        stream.out_uint8(0); /* tri blt */
        stream.out_uint8(0); /* unused */
        stream.out_uint8(0); /* unused */
        stream.out_uint8(0); /* nine grid */
        stream.out_uint8(1); /* line to */
        stream.out_uint8(0); /* multi nine grid */
        stream.out_uint8(1); /* rect */
        stream.out_uint8(0); /* desk save */
        stream.out_uint8(0); /* unused */
        stream.out_uint8(0); /* unused */
        stream.out_uint8(0); /* unused */
        stream.out_uint8(0); /* multi dest blt */
        stream.out_uint8(0); /* multi pat blt */
        stream.out_uint8(0); /* multi screen blt */
        stream.out_uint8(0); /* multi rect */
        stream.out_uint8(0); /* fast index */
        stream.out_uint8(0); /* polygon */
        stream.out_uint8(0); /* polygon */
        stream.out_uint8(0); /* polyline */
        stream.out_uint8(0); /* unused */
        stream.out_uint8(0); /* fast glyph */
        stream.out_uint8(0); /* ellipse */
        stream.out_uint8(0); /* ellipse */
        stream.out_uint8(0); /* ? */
        stream.out_uint8(0); /* unused */
        stream.out_uint8(0); /* unused */
        stream.out_uint8(0); /* unused */
        stream.out_uint8(0); /* unused */
        stream.out_uint16_le(0x6a1);
        stream.out_clear_bytes(2); /* ? */
        stream.out_uint32_le(0x0f4240); /* desk save */
        stream.out_uint32_le(0x0f4240); /* desk save */
        stream.out_uint32_le(1); /* ? */
        stream.out_uint32_le(0); /* ? */

        /* Output color cache capability set */
        caps_count++;
        stream.out_uint16_le(RDP_CAPSET_COLCACHE);
        stream.out_uint16_le(RDP_CAPLEN_COLCACHE);
        stream.out_uint16_le(6); /* cache size */
        stream.out_uint16_le(0); /* pad */

        /* Output pointer capability set */
        caps_count++;
        stream.out_uint16_le(RDP_CAPSET_POINTER);
        stream.out_uint16_le(RDP_CAPLEN_POINTER);
        stream.out_uint16_le(1); /* Colour pointer */
        stream.out_uint16_le(0x19); /* Cache size */
        stream.out_uint16_le(0x19); /* Cache size */

        /* Output input capability set */
        caps_count++;
        stream.out_uint16_le(RDP_CAPSET_INPUT); /* 13(0xd) */
        stream.out_uint16_le(RDP_CAPLEN_INPUT); /* 88(0x58) */
        stream.out_uint8(1);
        stream.out_clear_bytes(83);

        stream.out_clear_bytes(4); /* pad */

        size_t caps_size = stream.p - caps_ptr;
        caps_size_ptr[0] = caps_size;
        caps_size_ptr[1] = caps_size >> 8;

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

    /*****************************************************************************/
    void capset_general(Stream & stream, int len)
    {
        LOG(LOG_INFO, "capset_general");
        stream.skip_uint8(10);
        /* use_compact_packets is pretty much 'use rdp5' */
        this->client_info.use_compact_packets = stream.in_uint16_le();
        if (this->client_info.use_compact_packets){
            LOG(LOG_INFO, "Use compact packets");
        }
        /* op2 is a boolean to use compact bitmap headers in bitmap cache */
        /* set it to same as 'use rdp5' boolean */
        this->client_info.op2 = this->client_info.use_compact_packets;
        if (this->client_info.op2){
            LOG(LOG_INFO, "Use compact headers for cache");
        }
    }

    /*****************************************************************************/
    void capset_order(Stream & stream, int len)
    {
        LOG(LOG_INFO, "capset_order");
        stream.skip_uint8(20); /* Terminal desc, pad */
        stream.skip_uint8(2); /* Cache X granularity */
        stream.skip_uint8(2); /* Cache Y granularity */
        stream.skip_uint8(2); /* Pad */
        stream.skip_uint8(2); /* Max order level */
        stream.skip_uint8(2); /* Number of fonts */
        stream.skip_uint8(2); /* Capability flags */
        char order_caps[32];
        memcpy(order_caps, stream.in_uint8p(32), 32); /* Orders supported */
        LOG(LOG_INFO, "dest blt-0 %d\n", order_caps[0]);
        LOG(LOG_INFO, "pat blt-1 %d\n", order_caps[1]);
        LOG(LOG_INFO, "screen blt-2 %d\n", order_caps[2]);
        LOG(LOG_INFO, "memblt-3-13 %d %d\n", order_caps[3], order_caps[13]);
        LOG(LOG_INFO, "triblt-4-14 %d %d\n", order_caps[4], order_caps[14]);
        LOG(LOG_INFO, "line-8 %d\n", order_caps[8]);
        LOG(LOG_INFO, "line-9 %d\n", order_caps[9]);
        LOG(LOG_INFO, "rect-10 %d\n", order_caps[10]);
        LOG(LOG_INFO, "desksave-11 %d\n", order_caps[11]);
        LOG(LOG_INFO, "polygon-20 %d\n", order_caps[20]);
        LOG(LOG_INFO, "polygon2-21 %d\n", order_caps[21]);
        LOG(LOG_INFO, "polyline-22 %d\n", order_caps[22]);
        LOG(LOG_INFO, "ellipse-25 %d\n", order_caps[25]);
        LOG(LOG_INFO, "ellipse2-26 %d\n", order_caps[26]);
        LOG(LOG_INFO, "text2-27 %d\n", order_caps[27]);
        LOG(LOG_INFO, "order_caps dump\n");
        stream.skip_uint8(2); /* Text capability flags */
        stream.skip_uint8(6); /* Pad */
        /* desktop cache size, usually 0x38400 */
        this->client_info.desktop_cache = stream.in_uint32_le();;
        LOG(LOG_INFO, "desktop cache size %d\n", this->client_info.desktop_cache);
        stream.skip_uint8(4); /* Unknown */
        stream.skip_uint8(4); /* Unknown */
    }

    /* store the bitmap cache size in client_info */
    void capset_bmpcache(Stream & stream, int len)
    {
        LOG(LOG_INFO, "capset_bmpcache");
        stream.skip_uint8(24);
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
        stream.skip_uint8(2); /* number of caches in set, 3 */
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
        stream.skip_uint8(4); /* rdp_shareid */
        stream.skip_uint8(2); /* userid */
        int source_len = stream.in_uint16_le(); /* sizeof RDP_SOURCE */
        // int cap_len = stream.in_uint16_le();
        stream.skip_uint8(2); // skip cap_len
        stream.skip_uint8(source_len);
        int num_caps = stream.in_uint16_le();
        stream.skip_uint8(2); /* pad */

        for (int index = 0; index < num_caps; index++) {
            uint8_t *p = stream.p;
            int type = stream.in_uint16_le();
            int len = stream.in_uint16_le();

            switch (type) {
            case RDP_CAPSET_GENERAL: /* 1 */
                this->capset_general(stream, len);
                break;
            case RDP_CAPSET_BITMAP: /* 2 */
                break;
            case RDP_CAPSET_ORDER: /* 3 */
                this->capset_order(stream, len);
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
                    stream.skip_uint8(2); /* color pointer */
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

    #warning duplicated code in mod/rdp
    void send_synchronize()
    {
        LOG(LOG_INFO, "send_synchronize");

        Stream stream(8192);
        X224Out tpdu(X224Packet::DT_TPDU, stream);
        McsOut sdin_out(stream, MCS_SDIN, this->userid, MCS_GLOBAL_CHANNEL);
        SecOut sec_out(stream, this->client_info.crypt_level, SEC_ENCRYPT, this->encrypt);
        ShareControlOut rdp_control_out(stream, PDUTYPE_DATAPDU, this->userid + MCS_USERCHANNEL_BASE);
        ShareDataOut rdp_data_out(stream, PDUTYPE2_SYNCHRONIZE, this->share_id);

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
        LOG(LOG_INFO, "send_control");

        Stream stream(8192);
        X224Out tpdu(X224Packet::DT_TPDU, stream);
        McsOut sdin_out(stream, MCS_SDIN, this->userid, MCS_GLOBAL_CHANNEL);
        SecOut sec_out(stream, this->client_info.crypt_level, SEC_ENCRYPT, this->encrypt);
        ShareControlOut rdp_control_out(stream, PDUTYPE_DATAPDU, this->userid + MCS_USERCHANNEL_BASE);
        ShareDataOut rdp_data_out(stream, PDUTYPE2_CONTROL, this->share_id);

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

        #warning we should create some RDPStream object created on init and sent before destruction
        Stream stream(8192);
        X224Out tpdu(X224Packet::DT_TPDU, stream);
        McsOut sdin_out(stream, MCS_SDIN, this->userid, MCS_GLOBAL_CHANNEL);
        SecOut sec_out(stream, this->client_info.crypt_level, SEC_ENCRYPT, this->encrypt);
        ShareControlOut rdp_control_out(stream, PDUTYPE_DATAPDU, this->userid + MCS_USERCHANNEL_BASE);
        ShareDataOut rdp_data_out(stream, PDUTYPE2_FONTMAP, this->share_id);

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
        stream.skip_uint8(6);
        stream.in_uint16_le(); // len
        int data_type = stream.in_uint8();
        stream.in_uint8(); // ctype
        stream.in_uint16_le(); // clen
        switch (data_type) {
        case PDUTYPE2_POINTER: /* 27(0x1b) */
            LOG(LOG_INFO, "PDUTYPE2_POINTER");
            break;
        case PDUTYPE2_INPUT: /* 28(0x1c) */
            {
                int num_events = stream.in_uint16_le();
                stream.skip_uint8(2); /* pad */
                for (int index = 0; index < num_events; index++) {
                    int time = stream.in_uint32_le();
                    uint16_t msg_type = stream.in_uint16_le();
                    uint16_t device_flags = stream.in_uint16_le();
                    int16_t param1 = stream.in_sint16_le();
                    int16_t param2 = stream.in_sint16_le();

                    #warning we should always call send_input with original data, if the other side is rdp it will merely transmit it to the other end without change. If the other side is some internal module it will be it's own responsibility to decode it
                    #warning with the scheme above, any kind of keymap management is only necessary for internal modules or if we convert mapping. But only the back-end module really knows what the target mapping should be.
                    switch (msg_type) {
                    case RDP_INPUT_SYNCHRONIZE:
                        /* happens when client gets focus and sends key modifier info */
                        cb.set_key_flags(param1);
                        cb.rdp_input_synchronize(time, device_flags, param1, param2);
                        break;
                    case RDP_INPUT_SCANCODE:
                        {
                            long p1 = param1 % 128;
                            this->keys[p1] = 1 | device_flags;
                            if ((device_flags & KBD_FLAG_UP) == 0) { /* 0x8000 */
                                /* key down */
                                switch (p1) {
                                case 58:
                                    this->key_flags ^= 4;
                                    break; /* caps lock */
                                case 69:
                                    this->key_flags ^= 2;
                                    break; /* num lock */
                                case 70:
                                    this->key_flags ^= 1;
                                    break; /* scroll lock */
                                default:
                                    ;
                                }
                            }
                            struct key_info* ki = this->keymap->get_key_info_from_scan_code(
                                device_flags,
                                param1,
                                this->keys,
                                this->key_flags);
                            cb.rdp_input_scancode(param1, param2, device_flags, time, this->key_flags, this->keys, ki);
                            if (device_flags & KBD_FLAG_UP){
                                this->keys[p1] = 0;
                            }
                        }
                        break;
                    case RDP_INPUT_MOUSE:
                        this->mouse_x = param1;
                        this->mouse_y = param2;
                        cb.rdp_input_mouse(device_flags, param1, param2, this->key_flags, this->keys);
                        break;
                    default:
                        LOG(LOG_INFO, "unsupported PDUTYPE2_INPUT msg %u", msg_type);
                        break;
                    }
                }
            }
            break;
        case PDUTYPE2_CONTROL: /* 20(0x14) */
            LOG(LOG_INFO, "PDUTYPE2_CONTROL");
            {
                int action = stream.in_uint16_le();
                stream.skip_uint8(2); /* user id */
                stream.skip_uint8(4); /* control id */
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
        case PDUTYPE2_SYNCHRONIZE:
            LOG(LOG_INFO, "PDUTYPE2_SYNCHRONIZE");
            this->send_synchronize();
            this->up_and_running = 1;
            break;
        case PDUTYPE2_REFRESH_RECT:
            LOG(LOG_INFO, "PDUTYPE2_REFRESH_RECT");
            {
                /* int op = */ stream.in_uint32_le();
                int left = stream.in_uint16_le();
                int top = stream.in_uint16_le();
                int right = stream.in_uint16_le();
                int bottom = stream.in_uint16_le();
                int cx = (right - left) + 1;
                int cy = (bottom - top) + 1;
                cb.rdp_input_invalidate(Rect(left, top, cx, cy));
            }
            break;
        case PDUTYPE2_SUPPRESS_OUTPUT:
            LOG(LOG_INFO, "PDUTYPE2_SUPPRESS_OUTPUT");
            // PDUTYPE2_SUPPRESS_OUTPUT comes when minimizing a full screen
            // mstsc.exe 2600. I think this is saying the client no longer wants
            // screen updates and it will issue a PDUTYPE2_REFRESH_RECT above
            // to catch up so minimized apps don't take bandwidth
            break;
        case PDUTYPE2_SHUTDOWN_REQUEST:
            LOG(LOG_INFO, "PDUTYPE2_SHUTDOWN_REQUEST");
            {
                // when this message comes, send a PDUTYPE2_SHUTDOWN_DENIED back
                // so the client is sure the connection is alive and it can ask
                // if user really wants to disconnect */
                Stream stream(8192);
                X224Out tpdu(X224Packet::DT_TPDU, stream);
                McsOut sdin_out(stream, MCS_SDIN, this->userid, MCS_GLOBAL_CHANNEL);
                SecOut sec_out(stream, this->client_info.crypt_level, SEC_ENCRYPT, this->encrypt);
                ShareControlOut rdp_control_out(stream, PDUTYPE_DATAPDU, this->userid + MCS_USERCHANNEL_BASE);
                ShareDataOut rdp_data_out(stream, PDUTYPE2_SHUTDOWN_DENIED, this->share_id);
                rdp_data_out.end();
                rdp_control_out.end();
                sec_out.end();
                sdin_out.end();
                tpdu.end();
                tpdu.send(this->trans);
            }
            break;
        case PDUTYPE2_FONTLIST: /* 39(0x27) */
//            LOG(LOG_INFO, "PDUTYPE2_FONTLIST");
            stream.skip_uint8(2); /* num of fonts */
            stream.skip_uint8(2); /* unknown */
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
            break;
        default:
            LOG(LOG_WARNING, "unsupported PDUTYPE in process_data %d\n", data_type);
            break;
        }
    }

    void send_deactive() throw (Error)
    {
        LOG(LOG_INFO, "send_deactive");
        Stream stream(8192);
        X224Out tpdu(X224Packet::DT_TPDU, stream);
        McsOut sdin_out(stream, MCS_SDIN, this->userid, MCS_GLOBAL_CHANNEL);
        SecOut sec_out(stream, this->client_info.crypt_level, SEC_ENCRYPT, this->encrypt);
        ShareControlOut(stream, PDUTYPE_DEACTIVATEALLPDU, this->userid + MCS_USERCHANNEL_BASE).end();
        sec_out.end();
        sdin_out.end();
        tpdu.end();
        tpdu.send(this->trans);
    }

};

#endif
