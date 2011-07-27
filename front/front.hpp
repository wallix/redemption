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
   Author(s): Christophe Grosjean, Javier Caverni, Xavier Dunat
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   header file. Front object, used to communicate with RDP client

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

#include "RDP/x224.hpp"
#include "server_rdp.hpp"

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

#include "transport.hpp"
#include "config.hpp"
#include "error.hpp"


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
    uint32_t offset_header;
    uint32_t offset_order_count;
    X224Out * tpdu;
    struct server_rdp & rdp_layer;

    GraphicsUpdatePDU(struct server_rdp & rdp_layer)
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
        offset_header(0),
        offset_order_count(0),
        rdp_layer(rdp_layer)
    {
        this->init();
    }


    void init(){
        this->stream.init(4096);
        this->tpdu = new X224Out(X224Packet::DT_TPDU, this->stream);

        stream.mcs_hdr = stream.p;
        stream.p += 8;

        if (this->rdp_layer.sec_layer.client_info->crypt_level > 1) {
            stream.sec_hdr = stream.p;
            stream.p += 4 + 8;
        }
        else {
            stream.sec_hdr = stream.p;
            stream.p += 4;
        }

        #warning we should define some kind of OrdersStream, to buffer in orders
        this->offset_header = this->stream.p - this->stream.data;

        this->stream.out_clear_bytes(18); // Share Header, we will fill it later

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

            stream.mark_end();
            this->rdp_layer.send_rdp_packet(this->stream, PDUTYPE_DATAPDU, PDUTYPE2_UPDATE, this->offset_header);
//            LOG(LOG_INFO, "server_sec_send front");
            this->rdp_layer.sec_layer.server_sec_send(stream, MCS_GLOBAL_CHANNEL);
            tpdu->end();
            tpdu->send(this->rdp_layer.sec_layer.trans);
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
    }

    void send(const RDPScrBlt & cmd, const Rect &clip)
    {
        this->reserve_order(25);
        RDPOrderCommon newcommon(RDP::SCREENBLT, clip);
        cmd.emit(this->stream, newcommon, this->common, this->scrblt);
        this->common = newcommon;
        this->scrblt = cmd;
    }

    void send(const RDPDestBlt & cmd, const Rect &clip)
    {
        this->reserve_order(21);
        RDPOrderCommon newcommon(RDP::DESTBLT, clip);
        cmd.emit(this->stream, newcommon, this->common, this->destblt);
        this->common = newcommon;
        this->destblt = cmd;
    }


    void send(const RDPPatBlt & cmd, const Rect &clip)
    {
        this->reserve_order(29);
        using namespace RDP;
        RDPOrderCommon newcommon(RDP::PATBLT, clip);
        cmd.emit(this->stream, newcommon, this->common, this->patblt);
        this->common = newcommon;
        this->patblt = cmd;
    }


    void send(const RDPMemBlt & cmd, const Rect & clip)
    {
        this->reserve_order(30);
        RDPOrderCommon newcommon(RDP::MEMBLT, clip);
        cmd.emit(this->stream, newcommon, this->common, this->memblt);
        this->common = newcommon;
        this->memblt = cmd;
    }

    void send(const RDPLineTo& cmd, const Rect & clip)
    {
        this->reserve_order(32);
        RDPOrderCommon newcommon(RDP::LINE, clip);
        cmd.emit(this->stream, newcommon, this->common, this->lineto);
        this->common = newcommon;
        this->lineto = cmd;
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
    }

    void send(const RDPColCache & cmd)
    {
        this->reserve_order(2000);
        cmd.emit(this->stream);
    }

    void send(const RDPBmpCache & cmd)
    {
        this->reserve_order(cmd.bmp->bmp_size(cmd.bpp) + 16);
        cmd.emit(this->stream);
    }

    void send(const RDPGlyphCache & cmd)
    {
        #warning compute actual size, instead of a majoration as below
        this->reserve_order(1000);
        cmd.emit(this->stream);
    }

};

#warning front is becoming an empty shell and should disappear soon
class Front {
public:
    Cache cache;
    struct BitmapCache *bmp_cache;
    struct Font font;
    int mouse_x;
    int mouse_y;
    bool nomouse;
    bool notimestamp;
    int timezone;

    struct server_rdp rdp_layer;
    int order_level;
    GraphicsUpdatePDU * orders;

public:

    Front(SocketTransport * trans, Inifile * ini) :
    cache(),
    bmp_cache(0),
    font(SHARE_PATH "/" DEFAULT_FONT_NAME),
    mouse_x(0),
    mouse_y(0),
    nomouse(ini->globals.nomouse),
    notimestamp(ini->globals.notimestamp),
    timezone(0),
    rdp_layer(trans, ini),
    order_level(0)
    {
        this->orders = new GraphicsUpdatePDU(rdp_layer);
    }

    ~Front(){
        if (this->bmp_cache){
            delete this->bmp_cache;
        }
    }

    const ClientInfo & get_client_info() const {
        return this->rdp_layer.client_info;
    }

    void set_client_info(uint16_t width, uint16_t height, uint8_t bpp)
    {
        this->rdp_layer.client_info.width = width;
        this->rdp_layer.client_info.height = height;
        this->rdp_layer.client_info.bpp = bpp;
    }

    void reset(){
//        LOG(LOG_INFO, "reset()");
        #warning is it necessary (or even useful) to send remaining drawing orders before resetting ?
        this->orders->flush();

        /* shut down the rdp client */
        this->rdp_layer.server_rdp_send_deactive();

        /* this should do the resizing */
        this->rdp_layer.server_rdp_send_demand_active();

        delete this->orders;
        this->orders = new GraphicsUpdatePDU(this->rdp_layer);

        #warning some duplication of front instanciation code, it probably means that current front is part of rdp layer, not the other way around, as rdp_layer has a longer lifetime than front. On the other hand caches seems to be on the same timeline that front (means caches are parts of front) and should not be instnaciated elsewhere.
        if (this->bmp_cache){
            delete this->bmp_cache;
        }
        this->bmp_cache = new BitmapCache(&(this->rdp_layer.client_info));
        this->cache.reset(this->rdp_layer.client_info);
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

    void send_pointer(int cache_idx, uint8_t* data, uint8_t* mask, int x, int y) throw (Error)
    {
//        LOG(LOG_INFO, "front::send_pointer\n");
        this->rdp_layer.server_rdp_send_pointer(cache_idx, data, mask, x, y);
//        LOG(LOG_INFO, "front::send_pointer done\n");
    }

    void set_pointer(int cache_idx) throw (Error)
    {
//        LOG(LOG_INFO, "front::set_pointer\n");
        this->rdp_layer.server_rdp_set_pointer(cache_idx);
//        LOG(LOG_INFO, "front::set_pointer done\n");
    }

    void activate_and_process_data(Callback & cb)
    {
//        LOG(LOG_INFO, "activate_and_process_data\n");
        this->rdp_layer.activate_and_process_data(cb);
//        LOG(LOG_INFO, "activate_and_process_data done\n");
    }

    void disconnect() throw (Error)
    {
//        LOG(LOG_INFO, "disconnect()");
        this->rdp_layer.server_rdp_disconnect();
//        LOG(LOG_INFO, "disconnect() done");
    }

    void set_console_session(bool b)
    {
//        LOG(LOG_INFO, "set_console_session");
        this->rdp_layer.client_info.console_session = b;
//        LOG(LOG_INFO, "set_console_session done");
    }

    const vector<struct mcs_channel_item *> & get_channel_list(void) const
    {
//        LOG(LOG_INFO, "get_channel_list()");
        return rdp_layer.sec_layer.mcs_layer.channel_list;
//        LOG(LOG_INFO, "get_channel_list() done");
    }

    void incoming(void)
    {
//        LOG(LOG_INFO, "incoming");
        this->rdp_layer.server_rdp_incoming();
//        LOG(LOG_INFO, "incoming done");
    }

    void send_global_palette(const BGRPalette & palette)
    {
//        LOG(LOG_INFO, "send_global_palette()");
        this->rdp_layer.send_global_palette(palette);
//        LOG(LOG_INFO, "send_global_palette() done");
    }

    int get_channel_id(char* name)
    {
//        LOG(LOG_INFO, "front::get_channel_id\n");
        return this->rdp_layer.sec_layer.mcs_layer.mcs_get_channel_id(name);
    }

    void send_to_channel(int channel_id,
                       uint8_t* data, int data_len,
                       int total_data_len, int flags)
    {
//        LOG(LOG_INFO, "send_to_channel()");
        this->rdp_layer.server_send_to_channel(channel_id, data, data_len, total_data_len, flags);
//        LOG(LOG_INFO, "send_to_channel() done");
    }

};

#endif
