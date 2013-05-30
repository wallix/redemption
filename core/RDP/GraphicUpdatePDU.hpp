/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2011
   Author(s): Christophe Grosjean, Raphael Zhou

   RDPGraphicDevice is an abstract class that describe a device able to
   proceed RDP Drawing Orders. How the drawing will be actually done
   depends on the implementation.
   - It may be sent on the wire,
   - Used to draw on some internal bitmap,
   - etc.
*/

#ifndef _REDEMPTION_CORE_RDP_GRAPHIC_UPDATE_PDU_HPP_
#define _REDEMPTION_CORE_RDP_GRAPHIC_UPDATE_PDU_HPP_

#include "RDP/x224.hpp"
#include "RDP/mcs.hpp"
#include "RDP/rdp.hpp"
#include "RDP/sec.hpp"
#include "RDP/lic.hpp"
#include "RDP/RDPGraphicDevice.hpp"
#include "RDP/fastpath.hpp"

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

struct GraphicsUpdatePDU : public RDPSerializer
{
    BStream buffer_stream_orders;
    BStream buffer_stream_bitmaps;
    ShareData * sdata;
    uint16_t & userid;
    int & shareid;
    int & encryptionLevel;
    CryptContext & encrypt;
    bool fastpath_support;

    GraphicsUpdatePDU(Transport * trans,
                      uint16_t & userid,
                      int & shareid,
                      int & encryptionLevel,
                      CryptContext & encrypt,
                      const Inifile & ini,
                      const uint8_t bpp,
                      BmpCache & bmp_cache,
                      const int bitmap_cache_version,
                      const int use_bitmap_comp,
                      const int op2,
                      bool fastpath_support)
        : RDPSerializer( trans, this->buffer_stream_orders
                       , this->buffer_stream_bitmaps, bpp, bmp_cache
                       , bitmap_cache_version, use_bitmap_comp, op2, ini)
        , buffer_stream_orders(65536)
        , buffer_stream_bitmaps(65536)
        , sdata(NULL)
        , userid(userid)
        , shareid(shareid)
        , encryptionLevel(encryptionLevel)
        , encrypt(encrypt)
        , fastpath_support(fastpath_support)
    {
        this->init();
    }

    ~GraphicsUpdatePDU(){
        if (this->sdata){ delete this->sdata; }
    }

    void init(){
        if (this->fastpath_support == false) {
            if (this->sdata){ delete this->sdata; }
            this->sdata = new ShareData(this->stream_orders);

            if (this->ini.globals.debug.primary_orders > 3){
                LOG(LOG_INFO, "GraphicsUpdatePDU::init::Initializing orders batch mcs_userid=%u shareid=%u", this->userid, this->shareid);
            }
            this->sdata->emit_begin(PDUTYPE2_UPDATE, this->shareid, RDP::STREAM_MED);
            TODO("this is to kind of header, to be treated like other headers")
            this->stream_orders.out_uint16_le(RDP_UPDATE_ORDERS);
            this->stream_orders.out_clear_bytes(2); /* pad */
            this->offset_order_count = this->stream_orders.get_offset();
            this->stream_orders.out_clear_bytes(2); /* number of orders, set later */
            this->stream_orders.out_clear_bytes(2); /* pad */
        }
        else {
            this->stream_orders.out_clear_bytes(FastPath::Update_Send::GetSize()); // Fast-Path Update (TS_FP_UPDATE structure) size
            this->offset_order_count = this->stream_orders.get_offset();
            this->stream_orders.out_clear_bytes(2);  // number of orders, set later
        }

        // Slow-path
//        this->buffer_stream_bitmaps
    }

    virtual void flush_orders()
    {
        if (this->order_count > 0){
            if (this->ini.globals.debug.primary_orders > 3){
                LOG(LOG_INFO, "GraphicsUpdatePDU::flush_orders: order_count=%d offset=%u", this->order_count, this->offset_order_count);
            }
            this->stream_orders.set_out_uint16_le(this->order_count, this->offset_order_count);

            if (this->fastpath_support == false) {
                if (this->ini.globals.debug.primary_orders > 3){
                    LOG(LOG_INFO, "GraphicsUpdatePDU::flush_orders:slow-path");
                }

                this->sdata->emit_end();

                BStream sctrl_header(256);
                ShareControl_Send(sctrl_header, PDUTYPE_DATAPDU, this->userid + GCC::MCS_USERCHANNEL_BASE, this->stream_orders.size());

                BStream target_stream(65536);
                target_stream.out_copy_bytes(sctrl_header);
                target_stream.out_copy_bytes(this->stream_orders);
                target_stream.mark_end();

                BStream x224_header(256);
                BStream mcs_header(256);
                BStream sec_header(256);

                SEC::Sec_Send sec(sec_header, target_stream, 0, this->encrypt, this->encryptionLevel);
                MCS::SendDataIndication_Send mcs(mcs_header, this->userid, GCC::MCS_GLOBAL_CHANNEL, 1, 3, sec_header.size() + target_stream.size(), MCS::PER_ENCODING);
                X224::DT_TPDU_Send(x224_header, mcs_header.size() + sec_header.size() + target_stream.size());
                this->trans->send(x224_header, mcs_header, sec_header, target_stream);
            }
            else {
                if (this->ini.globals.debug.primary_orders > 3){
                    LOG(LOG_INFO, "GraphicsUpdatePDU::flush_orders: fast-path");
                }

                this->stream_orders.mark_end();
                SubStream Upd_s(this->stream_orders, 0, FastPath::Update_Send::GetSize());

                FastPath::Update_Send Upd( Upd_s
                                         , this->stream_orders.size() - FastPath::Update_Send::GetSize()
                                         , FastPath::FASTPATH_UPDATETYPE_ORDERS
                                         , FastPath::FASTPATH_FRAGMENT_SINGLE);

                BStream fastpath_header(256);

                FastPath::ServerUpdatePDU_Send SvrUpdPDU(
                      fastpath_header
                    , this->stream_orders
                    , ((this->encryptionLevel > 1) ? FastPath::FASTPATH_OUTPUT_ENCRYPTED : 0)
                    , this->encrypt
                    );

                this->trans->send(fastpath_header, this->stream_orders);
            }

            this->order_count = 0;
            this->stream_orders.reset();
            this->init();
        }
    }

    virtual void flush_bitmaps() {
        if (this->bitmap_count > 0) {
            LOG(LOG_INFO, "GraphicsUpdatePDU::flush_bitmap: bitmap_count=%d offset=%u", this->bitmap_count, this->offset_order_count);
        }
    }
};

#endif
